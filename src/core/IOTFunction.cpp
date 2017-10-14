/*
** EasyIOT - Device Function Class
**
** This program is provided free for you to use in any way that you wish,
** subject to the laws and regulations where you are using it.  Due diligence
** is strongly suggested before using this code.  Please give credit where due.
**
** The Author makes no warranty of any kind, express or implied, with regard
** to this program or the documentation contained in this document.  The
** Author shall not be liable in any event for incidental or consequential
** damages in connection with, or arising out of, the furnishing, performance
** or use of these programs.
*/
#include "IOTProperty.h"
#include "IOTFunction.h"
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>

/*
** Default NULL Property
*/
IOTProperty *IOTFunction::_nullProperty = NULL;

/*
** Function Class Construction
*/
IOTFunction::IOTFunction(const char *tag, uint8_t numProperties)
    : _tag(tag),
      _nvsHandle(0), 
      _flags(0),
      _state(IOT_STOPPED), 
      _propCount(numProperties), _Properties(NULL)
{
    static uint32_t nullData = 0;

    if (_nullProperty == NULL)
        _nullProperty = new IOTPropertyNumber<uint32_t>(NULL, IOT_FLAG_READONLY, nullData, 0, 0, PROPERTY_CLASS::GENERIC);

    if (numProperties != 0)
    {
        _Properties = new IOTProperty *[numProperties];
        memset(_Properties, 0, sizeof(IOTProperty * [numProperties]));
    }
    
    ESP_LOGV(_tag, "Created Function (%d Properties)", numProperties);    
}

/*
** Function Class Destruction
*/
IOTFunction::~IOTFunction()
{
    if (_Properties != NULL)
    {
        for (int p = 0; p < _propCount; p++)
        {
            if (_Properties[p] != NULL)
                delete _Properties[p];
        }
        free(_Properties);
        _Properties = NULL;
    }

    if (_nvsHandle)
    {
        nvs_close(_nvsHandle);
        _nvsHandle = 0;
    }
}

IOTHTTP *IOTFunction::Server(void) const
{
    if (_iotMaster != nullptr)
        return _iotMaster->Server();
    return nullptr;
}

/*
** Find function by tag
*/
IOTFunction *IOTFunction::Function(const char * tag)
{
    if (tag != nullptr) {
        IOTFunction *func = listHead();

        if (strcmp(tag, _tag) == 0)
            return this;

        while (func != nullptr)
        {
            if (strcmp(tag, func->_tag) == 0)
                return func;
            func = func->_listNext;
        }
    }
    return nullptr;
}

/*
** Function List Heade and Tail pointers
*/
IOTFunction *IOTFunction::listHead(void) const
{
    if (_iotMaster != nullptr)
        return _iotMaster->listHead();
    return nullptr;    
}

IOTFunction *IOTFunction::listTail(void) const
{
    if (_iotMaster != nullptr)
        return _iotMaster->listTail();
    return nullptr;
}

/*
** Initialize Function
*/
void IOTFunction::_initFunction(void)
{
    if (!_nvsHandle)
    {
        ESP_LOGV(_tag, "Initialize Properties.");
        
        esp_err_t err;
        if ((err = nvs_open(_tag, NVS_READWRITE, &_nvsHandle)))
        {
            _nvsHandle = 0;
            ESP_LOGE(_tag, "nvs_open failed: %s", nvs_error(err));
        }
        
        if (!(_flags & IOT_FLAG_LOCK_LABEL)) {
            _label = _loadLabel(strLabel, _label, IOTFunction_MAX_LABEL);
            ESP_LOGD(_tag, "Loaded Label: %s", _label);
        }

        for (int p = 0; _Properties != nullptr && p < _propCount; p++)
            _loadProperty(_Properties[p]);
    }
}

/*
** Load/Save Property Data
*/
void IOTFunction::_loadProperty(IOTProperty *prop)
{
    for (uint8_t p = 0; _Properties != nullptr && prop != nullptr && p < _propCount; p++)
    {
        if (_Properties[p] == prop)
        {
            char key[16];
            
            if (!(prop->_dataFlags & IOT_FLAG_LOCK_LABEL))
            {
                sprintf(key, "%s@P%3.3d", strLabel, p);
                prop->_dataLabel = _loadLabel(key, prop->_dataLabel, IOTPROPERTY_MAX_LABEL);
            }

            // Time Stamp
            sprintf(key, "%s@P%.3d", strTime, p);
            (void)_loadBytes(key, &prop->_dataTime, sizeof(prop->_dataTime));

            // Value
            sprintf(key, "%s@P%3.3d", strValue, p);
            switch (prop->_dataType)
            {
            case PROPERTY_TYPE::STRING:
                (void)_loadChars(key, (char *)prop->_dataPtr(), prop->dataLen());
                break;
            default:
                (void)_loadBytes(key, prop->_dataPtr(), prop->dataLen());
                break;
            }

            ESP_LOGD(_tag, "Loaded Property %d (%s)", p, prop->getData().c_str());
            
            return;
        }
    }
}

void IOTFunction::_saveProperty(IOTProperty *prop)
{
    for (uint8_t p = 0; _Properties != nullptr && p < _propCount; p++)
    {
        if (_Properties[p] == prop)
        {
            char key[20];
            
            if (prop->_dataFlags & IOT_FLAG_VOLATILE) {
                ESP_LOGD(_tag, "Volatile Property %d (%s), save ignored.", p, prop->getData().c_str());
                return;
            }

            // Time Stamp
            sprintf(key, "%s@P%.3d", strTime, p);
            (void)_saveBytes(key, &prop->_dataTime, sizeof(prop->_dataTime));

            // Value
            sprintf(key, "%s@P%.3d", strValue, p);
            switch (prop->_dataType)
            {
            case PROPERTY_TYPE::STRING:
                (void)_saveChars(key, (char *)prop->_dataPtr());
                break;
            default:
                (void)_saveBytes(key, prop->_dataPtr(), prop->dataLen());
                break;
            }

            ESP_LOGD(_tag, "Saved Property %d (%s)", p, prop->getData().c_str());
            
            return;
        }
    }
}

/*
** Load/Save Label
*/
char *IOTFunction::_loadLabel(const char *key, char *label, size_t max)
{
    size_t len = 0;

    if ((_nvsHandle) && (len = _loadChars(key, NULL, max)) > 0)
    {
        len = std::min(len, max);
        if (label != NULL)
            free(label);
        if ((label = (char *)malloc(len + 1)) != NULL)
        {
            memset(label, 0, len + 1);
            if (_loadChars(key, label, len) != len)
            {
                free(label);
                return NULL;
            }
        }
        else
            ESP_LOGE(_tag, "malloc() failed: %s", key);
    }

    return label;
}

/*
** Load/Save Data Bytes
*/
size_t IOTFunction::_loadChars(const char *key, char *store, size_t max)
{
    if (!_nvsHandle)
        return 0;

    esp_err_t err;
    size_t len = 0;

    if ((err = nvs_get_str(_nvsHandle, key, NULL, &len)))
    {
        if (err != ESP_ERR_NVS_NOT_FOUND)
            ESP_LOGE(_tag, "nvs_get_str len fail: %s %s", key, nvs_error(err));
        return 0;
    }

    if (len > max)
    {
        ESP_LOGE(_tag, "not enough space in buffer: %u < %u", max, len);
        return 0;
    }

    if (store != NULL)
    {
        if ((err = nvs_get_str(_nvsHandle, key, store, &len)))
        {
            ESP_LOGE(_tag, "nvs_get_str fail: %s %s", key, nvs_error(err));
            return 0;
        }

        store[len] = '\0';
    }
    return len;
}

size_t IOTFunction::_saveChars(const char *key, char *store)
{
    if (!_nvsHandle)
        return 0;

    esp_err_t err;
    if ((err = nvs_set_str(_nvsHandle, key, store)))
    {
        ESP_LOGE(_tag, "nvs_set_str fail: %s %s", key, nvs_error(err));
        return 0;
    }

    if ((err = nvs_commit(_nvsHandle)))
    {
        ESP_LOGE(_tag, "nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }

    return strlen(store);
}

/*
** Load/Save Data Bytes
*/
size_t IOTFunction::_loadBytes(const char *key, void *store, size_t max)
{
    if (!_nvsHandle)
        return 0;

    esp_err_t err;
    size_t len = 0;

    if ((err = nvs_get_blob(_nvsHandle, key, NULL, &len)))
    {
        if (err != ESP_ERR_NVS_NOT_FOUND)
            ESP_LOGE(_tag, "nvs_get_blob len fail: %s %s", key, nvs_error(err));
        return 0;
    }

    if (len > max)
    {
        ESP_LOGE(_tag, "not enough space in buffer: %u < %u", max, len);
        return 0;
    }

    if ((err = nvs_get_blob(_nvsHandle, key, store, &len)))
    {
        ESP_LOGE(_tag, "nvs_get_blob fail: %s %s", key, nvs_error(err));
        return 0;
    }

    return len;
}

size_t IOTFunction::_saveBytes(const char *key, void *store, size_t len)
{
    if (!_nvsHandle)
        return 0;

    esp_err_t err;
    if ((err = nvs_set_blob(_nvsHandle, key, store, len)))
    {
        ESP_LOGE(_tag, "nvs_set_blob fail: %s %s", key, nvs_error(err));
        return 0;
    }

    if ((err = nvs_commit(_nvsHandle)))
    {
        ESP_LOGE(_tag, "nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }

    return len;
}

void IOTFunction::_postUpdate(uint8_t p, bool urgent)
{
    if (_Properties != nullptr && _propCount > 0 && p < _propCount)
    {
        if (_Properties[p] != nullptr)
            _postUpdate(_Properties[p], urgent);
    }
}

void IOTFunction::_postUpdate(IOTProperty *prop, bool urgent)
{
    if (prop == nullptr)
        return;

    time(&prop->_dataTime);

    if (!(prop->_dataFlags & IOT_FLAG_READONLY))
        _saveProperty(prop);

    // TODO: Call user onChange Handler, if any
}

/*
** Function Property Selector
*/
IOTProperty *IOTFunction::Property(uint8_t p)
{
    if (_Properties != nullptr && _propCount > 0 && p < _propCount)
    {
        if (_Properties[p] != nullptr)
            return _Properties[p];
    }
    _nullProperty->_IOTFunction = this;
    return _nullProperty;
}

/*
** Function Label
*/
const char *IOTFunction::getLabel(void) const
{
    return (_label != nullptr) ? _label : strNull;
}

void IOTFunction::getLabel(char *s)
{
    if (s != nullptr)
    {
        int n = 0;

        if (_label != nullptr)
        {
            n = IOTFunction_MAX_LABEL;
            strncpy(s, _label, n);
        }
        s[n] = '\0';
    }
}

void IOTFunction::setLabel(const char *s, bool lock)
{
    if (_flags & IOT_FLAG_LOCK_LABEL)
        return;

    if (_label != nullptr)
    {
        free(_label);
        _label = nullptr;
    }

    if (s != nullptr)
    {
        int n = strlen(s);
        if (n > 0)
        {
            n = std::min(n, IOTFunction_MAX_LABEL);
            if ((_label = (char *)malloc(n + 1)) != nullptr)
            {
                memcpy(_label, s, n);
                _label[n] = '\0';

                if (!lock)
                    (void)_saveChars(strLabel, _label);
            }
        }
    }

    if (lock)
        _flags |= IOT_FLAG_LOCK_LABEL;
}
/******************************************************************************/