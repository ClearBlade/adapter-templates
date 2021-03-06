# Python Adapter Template

When creating adapters, there are tasks that adapters must accomplished in order to for the adapter to communicate with the ClearBlade Platform or ClearBlade Edge. The Python adapter template contains the logic needed to implement these commonly needed tasks. 


The python adapter template currently supports the following:
  1. Command line argument definition and validation
  2. Authenticating to the ClearBlade Platform or ClearBlade Edge as a device
  3. Optionally Retrieving adapter runtime configuration parameters from a ClearBlade Platform data collection
  4. Connecting to a ClearBlade Platform or ClearBlade Edge MQTT message broker with language specific MQTT callbacks
  5. Custom logging with log level filtering

## ClearBlade Platform Dependencies
The adapter template was constructed to provide the ability to communicate with a _System_ defined in a ClearBlade Platform instance. Therefore, the adapter template requires a _System_ to have been created within a ClearBlade Platform instance.

Once a System has been created, artifacts must be defined within the ClearBlade Platform system to allow the adapter template to function properly. At a minimum, a device needs to be created in the Auth --> Devices collection. The device will represent the adapter, or more importantly the IoT gateway, on which the adapter is executing. The _name_ and _active key_ values specified in the Auth --> Devices collection will be used by the adapter template to authenticate to the ClearBlade Platform or ClearBlade Edge. 

## Usage
The python adapter template was written to be compatible with both Python 2 and Python 3. When writing a Python adapter using the Python adapter template, the _adapter.py_ file should be copied to the filesystem directory where your source code resides. Additionally, you will want to rename the file so that the name gives an indication of the type of adapter it is. For example, zigbee_adapter.py

### Executing the adapter template

#### Python 2
`python adapter.py --systemKey=<PLATFORM SYSTEM KEY> --systemSecret=<PLATFORM SYSTEM SECRET> --deviceID=<AUTH DEVICE NAME> --activeKey=<AUTH DEVICE ACTIVE KEY> --httpUrl=<CB PLATFORM URL> --httpPort=<CB PLATFORM PORT> --messagingUrl=<CB PLATFORM MESSAGING URL> --messagingPort=<CB PLATFORM MESSAGING PORT> --adapterSettingsCollection=<CB DATA COLLECTION NAME> --adapterSettingsItem=<ROW ITEM ID VALUE> --topicRoot=<TOPIC ROOT> --deviceProvisionSvc=<PROVISIONING SERVICE NAME> --deviceHealthSvc=<HEALTH SERVICE NAME> --deviceLogsSvc=<DEVICE LOGS SERVICE NAME> --deviceStatusSvc=<DEVICE STATUS SERVICE NAME> --deviceDecommissionSvc=<DECOMMISSION SERVICE NAME> --logLevel=<LOG LEVEL> --logCB --logMQTT`

#### Python 3
`python3 adapter.py --systemKey=<PLATFORM SYSTEM KEY> --systemSecret=<PLATFORM SYSTEM SECRET> --deviceID=<AUTH DEVICE NAME> --activeKey=<AUTH DEVICE ACTIVE KEY> --httpUrl <CB PLATFORM URL> --httpPort=<CB PLATFORM PORT> --messagingUrl=<CB PLATFORM MESSAGING URL> --messagingPort=<CB PLATFORM MESSAGING PORT> --adapterSettingsCollection=<CB DATA COLLECTION NAME> --adapterSettingsItem=<ROW ITEM ID VALUE> --topicRoot=<TOPIC ROOT> --deviceProvisionSvc=<PROVISIONING SERVICE NAME> --deviceHealthSvc=<HEALTH SERVICE NAME> --deviceLogsSvc=<DEVICE LOGS SERVICE NAME> --deviceStatusSvc=<DEVICE STATUS SERVICE NAME> --deviceDecommissionSvc=<DECOMMISSION SERVICE NAME> --logLevel=<LOG LEVEL> --logCB --logMQTT`

   *Where* 

   __systemKey__
  * REQUIRED
  * The system key of the ClearBLade Platform __System__ the adapter will connect to

   __systemSecret__
  * REQUIRED
  * The system secret of the ClearBLade Platform __System__ the adapter will connect to
   
   __deviceID__
  * REQUIRED
  * The device name the BLE adapter will use to authenticate to the ClearBlade Platform
  * Requires the device to have been defined in the _Auth - Devices_ collection within the ClearBlade Platform __System__
   
   __activeKey__
  * REQUIRED
  * The active key the adapter will use to authenticate to the platform
  * Requires the device to have been defined in the _Auth - Devices_ collection within the ClearBlade Platform __System__
   
   __httpUrl__
  * The url (without the port number) of the ClearBlade Platform instance the adapter will connect to
  * OPTIONAL
  * Defaults to __http://localhost__

   __httpPort__
  * The port number of the ClearBlade Platform instance the adapter will connect to
  * OPTIONAL
  * Defaults to __9000__

   __messagingUrl__
  * The MQTT url (without the port number) of the ClearBlade Platform instance the adapter will connect to
  * OPTIONAL
  * Defaults to __localhost__

   __messagingPort__
  * The MQTT port number of the ClearBlade Platform instance the adapter will connect to
  * OPTIONAL
  * Defaults to __1883__

   __adapterSettingsCollection__
  * The MQTT port number of the ClearBlade Platform instance the adapter will connect to
  * See the _Runtime Configuration_ section below
  * OPTIONAL

   __adapterSettingsItem__
  * The MQTT port number of the ClearBlade Platform instance the adapter will connect to
  * See the _Runtime Configuration_ section below
  * OPTIONAL

   __topicRoot__
  * The root hierarchy of the MQTT topic tree that should be used when subscribing to MQTT topics or publishing to MQTT topics
  * OPTIONAL

   __deviceProvisionSvc__
  * The name of a service, defined within the ClearBlade Platform or ClearBlade Edge, the adapter can invoke to provision IoT devices on the ClearBlade Platform or ClearBlade Edge.
  * Dependent upon MQTT. The adapter will publish data to a MQTT topic on the platform message broker containing relevant device information whenever the adapter needs to provision an IoT device on the ClearBlade Platform or ClearBlade Edge.
  * __Implementation specific. Will need to be implemented by the developer.__
  * OPTIONAL

   __deviceHealthSvc__
  * The name of a service, defined within the ClearBlade Platform or ClearBlade Edge, the adapter can invoke to provide health information about connected IoT devices to the ClearBlade Platform or ClearBlade Edge.
  * Dependent upon MQTT. The adapter will publish data to a MQTT topic on the platform message broker containing relevant health information.
  * __Implementation specific. Will need to be implemented by the developer.__
  * OPTIONAL

   __deviceLogsSvc__
  * The name of a service, defined within the ClearBlade Platform or ClearBlade Edge, the adapter can invoke to provide device log entries to the ClearBlade Platform or ClearBlade Edge.
  * Dependent upon MQTT. The adapter will publish data to a MQTT topic on the platform message broker containing relevant log entries.
  * __Implementation specific. Will need to be implemented by the developer.__
  * OPTIONAL

   __deviceStatusSvc__
  * The name of a service, defined within the ClearBlade Platform or ClearBlade Edge, the adapter can invoke to provide the status of connected IoT devices to the ClearBlade Platform or ClearBlade Edge.
  * Dependent upon MQTT. The adapter will publish data to a MQTT topic on the platform message broker containing relevant device status information.
  * __Implementation specific. Will need to be implemented by the developer.__
  * OPTIONAL

   __deviceDecommissionSvc__
  * The name of a service, defined within the ClearBlade Platform or ClearBlade Edge, the adapter can invoke to decommission IoT devices.
  * Dependent upon MQTT. The adapter will publish a MQTT topic on the platform message broker containing relevant device information whenver the adapter needs to decommission an IoT device.
  * __Implementation specific. Will need to be implemented by the developer.__
  * OPTIONAL

   __logLevel__
  * The level of runtime logging the adapter should provide.
  * A developer can utilize the standard logging library provided by the python to add implemenation specific log information:
    * logging.critical
    * logging.error
    * logging.warning
    * logging.info
    * logging.debug
  * Available options are:
    * CRITICAL
    * ERROR
    * WARNING
    * INFO
    * DEBUG
  * OPTIONAL
  * Defaults to __INFO__

   __logCB__
  * Indicates whether or not log entries from the ClearBlade Python SDK should be printed
  * OPTIONAL
  * The presence of this command line argument indicates to the adapter that you wish to display ClearBlade SDK log information

   __logMQTT__
  * Indicates whether or not MQTT log entries should be printed
  * OPTIONAL
  * The presence of this command line argument indicates to the adapter that you wish to display MQTT log information


### Runtime Configuration
The adapter template includes the necessary code to allow for dynamic runtime configuration of an adapter. In order to utilize this capability, the _--adapterSettingsCollection_ command line argument must be supplied when the adapter is started. The value of the _--adapterSettingsCollection_ command line argument __MUST__ be the name of a data collection that has been defined on the ClearBlade Platform or ClearBlade Edge instance the adapter is connecting to. The structure and contents of the data collection are left to the developer to define.

By default, all rows in the data collection specified by the _--adapterSettingsCollection_ command line argument will be returned. If you wish to limit the data being returned to a specific row in the collection, include the _--adapterSettingsItem_ command line argument when the adapter is started. The value of the _--adapterSettingsItem_ argument __MUST__ be the value of the __item\_id__ column for a specific row in the data collection specified by the _--adapterSettingsCollection_ command line argument.

The get_adapter_config method in the adapter template contains a __TODO__ comment indicating where implementation logic should be placed.

## Setup
---
The python adapter template (and any python adapter) is dependent upon the ClearBlade Python SDK and its dependent libraries being installed. Follow the installation instructions at https://github.com/clearblade/clearblade-python-sdk to install the Python SDK.

## Todo
---
 - Add topic subscriptions to support the device _service_ command-line arguments:
   * deviceProvisionSvc
   * deviceHealthSvc
   * deviceLogsSvc
   * deviceStatusSvc
   * deviceDecommissionSvc

