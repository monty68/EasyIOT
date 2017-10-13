/*
** EasyIOT - Master Device Class
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
#include "EasyIOT.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <esp_system.h>
#include <esp_log.h>

/*
** General Defintions and Equates
*/
#define IOT_UUID_SERIAL "47843fea-e025-40bd-a38c-"

/*
** System State
*/
RTC_DATA_ATTR static int iotStatus = 0;

/*
** Construct EasyIOT
*/
IOTMaster::IOTMaster(const char *ssid, const char *pass, uint16_t port, bool lockWiFi)
    : IOTFunction("iot", 3),
      _webServer(nullptr),
      _needReboot(false)
{
    char host[IOT_MAX_HOST + 1];

    _chipID = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
    sprintf(host, "easyiot-%04X%08X", (uint16_t)(_chipID >> 32), (uint32_t)_chipID);
    sprintf(_uuid, "%s%04x%08x", IOT_UUID_SERIAL, (uint16_t)(_chipID >> 32), (uint32_t)_chipID);
    Serial.begin(115200);

    ESP_LOGV(_tag, "Default Hostname %s", host);
    uint16_t flags = IOT_FLAG_SYSTEM | IOT_FLAG_CONFIG | IOT_FLAG_LOCK_LABEL;

    if (lockWiFi)
        flags |= IOT_FLAG_READONLY;

    _Properties[0] = new IOTPropertyString(this, flags, ssid, IOT_MAX_SSID, PROPERTY_CLASS::GENERIC);
    _Properties[1] = new IOTPropertyString(this, flags, pass, IOT_MAX_PASS, PROPERTY_CLASS::GENERIC);
    _Properties[2] = new IOTPropertyString(this, flags, &host[0], IOT_MAX_HOST, PROPERTY_CLASS::IPHOST);

    /*
    //_Properties[3] = new IOTProperty<bool>(this, true, false, "Use DHCP");
    //_Properties[3] = new IOTProperty<bool>(this, 1, 0, 6, 1, PROPERTY_CLASS::CLASS_MODE, false);

    _Properties[4] = new IP4AddressProperty(this, INADDR_NONE, true, "IP Address");
    _Properties[5] = new IP4AddressProperty(this, INADDR_NONE, true, "Subnet Mask");
    _Properties[6] = new IP4AddressProperty(this, INADDR_NONE, true, "Gateway");
    _Properties[7] = new IP4AddressProperty(this, INADDR_NONE, true, "DNS #1");
    _Properties[8] = new IP4AddressProperty(this, INADDR_NONE, true, "DNS #2");
    */

    if ((_webServer = new IOTHTTP(_tag, port)) == NULL)
    {
        ESP_LOGE(_tag, "ERROR: failed to create web service.");
        iotReboot();
    }
}

/*
** Deconstruct EasyIOT
*/
IOTMaster::~IOTMaster()
{
    if (_webServer != NULL)
    {
        delete _webServer;
        _webServer = NULL;
    }
}

/*
** Reset (to defaults) the device
*/
void IOTMaster::sysReset(void)
{
    ESP_LOGI(_tag, "* System Reset *");
    iotShutdown();

    esp_err_t err;

    if (!_nvsHandle)
    {
        if ((err = nvs_open(_tag, NVS_READWRITE, &_nvsHandle)))
        {
            _nvsHandle = 0;
            ESP_LOGE(_tag, "nvs_open failed: %s", nvs_error(err));
        }
    }

    if (_nvsHandle)
    {
        ESP_LOGI(_tag, "Erasing NVS.");
        if ((err = nvs_erase_all(_nvsHandle)))
            ESP_LOGE(_tag, "nvs_erase_all fail: %s", nvs_error(err));
    }

    iotReboot();
}

/*
** Reboot the device
*/
void IOTMaster::sysReboot(void)
{
    ESP_LOGI(_tag, "* System Reboot *");

    for (int i = 10; i >= 0; i--)
    {
        ESP_LOGI(_tag, "Restarting in %d seconds...", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    iotShutdown();

    ESP_LOGI(_tag, "Restarting now.");
    
    if (_nvsHandle)
        nvs_close(_nvsHandle);
    _nvsHandle = 0;
    
    fflush(stdout);
    Serial.flush();
    Serial.end();
    esp_restart();
}

/*
** Startup the service
*/
void IOTMaster::iotStartup(void)
{
    ESP_LOGI(_tag, "Booting ...");

    ESP_LOGI(_tag, "Chip (%d): %04X%08X @ %d MHz - Flash: %d @ %d, SDK: %s",
             ESP.getChipRevision(),
             (uint16_t)(_chipID >> 32), (uint32_t)_chipID,
             ESP.getCpuFreqMHz(),
             ESP.getFlashChipSize(),
             ESP.getFlashChipSpeed(),
             ESP.getSdkVersion());

    ESP_LOGD(_tag, "Free Heap %d", ESP.getFreeHeap());

    // Start the flash driver
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    _initFunction();

    // Start WiFi
    WiFi.disconnect(true);

    ESP_LOGI(_tag, "Connecting WiFi.");
    WiFi.begin((char *)_Properties[0]->_dataPtr(), (char *)_Properties[1]->_dataPtr());
    WiFi.setHostname((char *)_Properties[2]->_dataPtr());

    int count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        yield();
        if (count++ > 30)
        {
            esp_restart();
        }
    }

    ESP_LOGI(_tag, "WiFi SSID  : %s", WiFi.SSID().c_str());
    ESP_LOGI(_tag, "Hostname   : %s", WiFi.getHostname());
    ESP_LOGI(_tag, "MAC Address: %s", WiFi.macAddress().c_str());
    ESP_LOGI(_tag, "IP Address : %s", WiFi.localIP().toString().c_str());
    ESP_LOGI(_tag, "Subnet Mask: %s", WiFi.subnetMask().toString().c_str());
    ESP_LOGI(_tag, "IP Gateway : %s", WiFi.gatewayIP().toString().c_str());
    ESP_LOGI(_tag, "DNS Server : %s", WiFi.dnsIP().toString().c_str());

    if (_webServer != NULL)
    {
        ESP_LOGI(_tag, "Starting mDNS Service.");        
        if (!MDNS.begin(WiFi.getHostname()))
        {
            ESP_LOGW(_tag, "Failed to start mDNS Service");
        }
        else
        {
            //MDNS.setInstanceName(getLabel());
            //MDNS.addService("_http", "_tcp", _webServer->webPort());
        }

        _webServer->webStartup();
    }

    _state = IOT_RUNNING;

    // Start the functions
    IOTFunction *func = listHead();

    while (func != NULL)
    {
        if (func->_state == IOT_STOPPED && !(func->_flags & IOT_FLAG_DISABLED))
        {
            ESP_LOGD(func->_tag, "Starting");
            func->_initFunction();
            func->iotStartup();
            if (func->_state != IOT_RUNNING)
                ESP_LOGW(func->_tag, "ERROR Starting: state: 0x%X", func->_state);
            func->_flags &= ~IOT_FLAG_RESTART;
        }
        func = func->_listNext;
    }

    MDNS.addService("_http", "_tcp", _webServer->webPort());
    ESP_LOGD(_tag, "Free Heap %d", ESP.getFreeHeap());
}

/*
** Shutdown the service
*/
void IOTMaster::iotShutdown(void)
{
    if (_state == IOT_STOPPED)
        return;

    // Shutdown functions (in reverse order of registration)
    IOTFunction *func = listTail();

    while (func != NULL)
    {
        if (func->_state == IOT_RUNNING)
        {
            func->iotShutdown();
            ESP_LOGI(func->_tag, "Stopped");
        }

        if (func->_nvsHandle)
            nvs_close(func->_nvsHandle);
        func->_nvsHandle = 0;
        func->_state = IOT_STOPPED;
        func = func->_listPrev;
    }

    if (_webServer != NULL)
        _webServer->webShutdown();

    // Shutdown WiFi
    if (WiFi.isConnected())
        WiFi.disconnect(true);
    _state = IOT_STOPPED;
    ESP_LOGI(_tag, "Stopped");
}

/*
** Feed the service
*/
void IOTMaster::iotService(void)
{
    if (_state != IOT_RUNNING)
        return;

    if (_webServer != nullptr)
        _webServer->webService();

    IOTFunction *func = listHead();

    while (func != nullptr && _state == IOT_RUNNING)
    {        
        if (func->_state == IOT_RUNNING && !(func->_flags & IOT_FLAG_DISABLED))
        {
            if (func->_flags & IOT_FLAG_RESTART) {
                ESP_LOGI(_tag, "Restarting");
                func->iotShutdown();
                func->iotStartup();
            }else
                func->iotService();
            yield();
        }
        func = func->_listNext;
    }

    if (_needReboot)
        sysReboot();
}

/*
** Restart the service
*/
void IOTMaster::iotRestart(void)
{
    if (_state != IOT_STOPPED)
        iotShutdown();
    iotStartup();
}

/*
** Master (web) Service
*/
IOTHTTP *IOTMaster::Server(void) const
{
    return _webServer;
}

/*
** Add new function
*/
void IOTMaster::addFunction(IOTFunction *func)
{
    if ((!func) || func == this || func->_iotMaster == this)
        return;
    
    IOTFunction *test = Function(func->_tag);
    if (test == nullptr) {
        ESP_LOGV(_tag, "addFunction(%s)", func->_tag);
        func->_iotMaster = this;
        listAppend(func);
    }else
        ESP_LOGE(_tag, "addFunction(%s) - Error, duplicate tag", func->_tag);    
}

/*
** List Handlers
*/
IOTFunction *IOTMaster::listHead(void) const { 
    return _listPrev;
}

IOTFunction *IOTMaster::listTail(void) const { 
    return _listNext;
}

void IOTMaster::listHead(IOTFunction *head) { 
    _listPrev = head; 
}

void IOTMaster::listTail(IOTFunction *tail) {
    _listNext = tail; 
}

void IOTMaster::listInsert(IOTFunction *func, IOTFunction *pSibling)
{
    // TODO - Not Sure we need this! ;-)
}

void IOTMaster::listAppend(IOTFunction *func)
{
    if (listHead() == nullptr)
    {
        func->_listNext = nullptr;
        func->_listPrev = nullptr;
        listHead(func);
        listTail(func);
    }
    else
    {
        func->_listNext = nullptr;
        func->_listPrev = listTail();
        func->_listPrev->_listNext = func;
        listTail(func);
    }
}

void IOTMaster::listRemove(IOTFunction *func)
{
    /* base case */
    if (!func || listHead() == nullptr)
        return;

    /* If node to be deleted is head node */
    if (listHead() == func)
        listHead(func->_listNext);

    /* If node to be deleted is tail node */
    if (listTail() == func)
        listTail(func->_listPrev);

    /* Change next only if node to be deleted is NOT the last node */
    if (func->_listNext != nullptr)
        func->_listNext->_listPrev = func->_listPrev;

    /* Change prev only if node to be deleted is NOT the first node */
    if (func->_listPrev != nullptr)
        func->_listPrev->_listNext = func->_listNext;
}

/*
** Master Class Property Updated
*/
bool IOTMaster::_propUpdate(IOTProperty *prop)
{
    return true;
}
/******************************************************************************/