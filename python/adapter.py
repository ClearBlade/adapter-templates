"""Adapter docstring
"""
import sys
import argparse
import logging
import os
from clearblade.ClearBladeCore import System, Query
from clearblade.ClearBladeCore import cbLogs

#TODO Change the name of the adapter to a name associated with your implementation
ADAPTER_NAME = "MyAdapter"
CB_CONFIG = {}


def parse_env_variables(env):
    """Parse environment variables"""
    possible_vars = ["CB_SYSTEM_KEY", "CB_SYSTEM_SECRET", "CB_EDGE_NAME", "CB_PLATFORM_IP", "CB_EDGE_IP", "CB_ADAPTERS_ROOT_DIR", "CB_SERVICE_ACCOUNT", "CB_SERVICE_ACCOUNT_TOKEN"]
    
    for var in possible_vars:
        if var in env:
            print("Setting config from environment variable: " + var)
            CB_CONFIG[var] = env[var]

    #TODO Add implementation specific environment variables here


def parse_args(argv):
    """Parse the command line arguments"""

    parser = argparse.ArgumentParser(description='ClearBlade Adapter')
    parser.add_argument('-systemKey', dest="CB_SYSTEM_KEY", help='The System Key of the ClearBlade \
                        Plaform "System" the adapter will connect to.')

    parser.add_argument('-systemSecret', dest="CB_SYSTEM_SECRET", help='The System Secret of the \
                        ClearBlade Plaform "System" the adapter will connect to.')

    parser.add_argument('-deviceID', dest="deviceID", help='The id/name of the device that will be used for device \
                        authentication against the ClearBlade Platform or Edge, defined \
                        within the devices table of the ClearBlade platform.')

    parser.add_argument('-activeKey', dest="activeKey", help='The active key of the device that will be used for device \
                        authentication against the ClearBlade Platform or Edge, defined within \
                        the devices table of the ClearBlade platform.')

    parser.add_argument('-cb_service_account', dest="CB_SERVICE_ACCOUNT", help='The id/name of the device service accountthat will be used for \
                        authentication against the ClearBlade Platform or Edge, defined \
                        within the devices table of the ClearBlade platform.')

    parser.add_argument('-cb_service_account_token', dest="CB_SERVICE_ACCOUNT_TOKEN", help='The token of the device service account that will be used for device \
                        authentication against the ClearBlade Platform or Edge, defined within \
                        the devices table of the ClearBlade platform.')

    parser.add_argument('-httpUrl', dest="httpURL", default="http://localhost", \
                        help='The HTTP URL of the ClearBlade Platform or Edge the adapter will \
                        connect to. The default is https://localhost.')

    parser.add_argument('-httpPort', dest="httpPort", default="9000", \
                        help='The HTTP Port of the ClearBlade Platform or Edge the adapter will \
                        connect to. The default is 9000.')

    parser.add_argument('-messagingUrl', dest="messagingURL", default="localhost", \
                        help='The MQTT URL of the ClearBlade Platform or Edge the adapter will \
                        connect to. The default is https://localhost.')

    parser.add_argument('-messagingPort', dest="messagingPort", type=int, default=1883, \
                        help='The MQTT Port of the ClearBlade Platform or Edge the adapter will \
                        connect to. The default is 9000.')

    parser.add_argument('-adapterSettingsCollection', dest="adapterSettingsCollectionName", \
                        default="", \
                        help='The name of the ClearBlade Platform data collection which contains \
                        runtime configuration settings for the adapter. The default is "".')

    parser.add_argument('-adapterSettingsItem', dest="adapterSettingsItemID", default="", \
                        help='The "item_id" of the row, within the ClearBlade Platform data \
                        collection which contains runtime configuration settings, that should \
                        be used to configure the adapter. The default is "".')

    parser.add_argument('-topicRoot', dest="adapterTopicRoot", default="", \
                        help='The root of MQTT topics this adapter will subscribe and publish to. \
                        The default is "".')

    parser.add_argument('-logLevel', dest="logLevel", default="INFO", choices=['CRITICAL', \
                        'ERROR', 'WARNING', 'INFO', 'DEBUG'], help='The level of logging that \
                        should be utilized by the adapter. The default is "INFO".')

    parser.add_argument('-logCB', dest="logCB", default=False, action='store_true',\
                        help='Flag presence indicates logging information should be printed for \
                        ClearBlade libraries.')

    parser.add_argument('-logMQTT', dest="logMQTT", default=False, action='store_true',\
                        help='Flag presence indicates MQTT logs should be printed.')

    #TODO Add implementation specific command line arguments here

    args = vars(parser.parse_args(args=argv[1:]))
    for var in args:
        if args[var] != "" and args[var] != None:
            print("Setting config from command line argument: " + var)
            CB_CONFIG[var] = args[var]


def check_required_config():
    """Verify all required config options were provided via environment variables or command line arguments"""
    if "CB_SYSTEM_KEY" not in CB_CONFIG:
        logging.error("System Key is required, can be provided with CB_SYSTEM_KEY environment variable or --systemKey command line argument")
        exit(-1)
    if not "CB_SYSTEM_SECRET" in CB_CONFIG:
        logging.error("System Secret is required, can be provided with CB_SYSTEM_SECRET environment variable or --systemSecret command line argument")
        exit(-1)

    if "deviceID" in CB_CONFIG and CB_CONFIG["deviceID"] != "" and CB_CONFIG["deviceID"] != None:
        if "activeKey" not in CB_CONFIG:
            logging.error("Device Active Key is required when a deviceID is specified, can be provided with the --activeKey command line argument")
            exit(-1)
    elif "CB_SERVICE_ACCOUNT" in CB_CONFIG and CB_CONFIG["CB_SERVICE_ACCOUNT"] != "" and CB_CONFIG["CB_SERVICE_ACCOUNT"] != None:
        if "CB_SERVICE_ACCOUNT_TOKEN" not in CB_CONFIG:
            logging.error("Device Service Account Token is required when a Service Account is specified, can be provided with the CB_SERVICE_ACCOUNT_TOKEN enviornment variable or --cb_service_account_token command line argument")
            exit(-1)
    else:
        logging.error("Device ID/Active Key or Service Account Name and Token are required")
        exit(-1)
    logging.debug("Adapter Config Looks Good!")
    
    

def setup_custom_logger(name):
    """Create a custom logger"""
    #logging.debug("Begin setup_custom_logger")

    formatter = logging.Formatter(fmt='%(asctime)s %(levelname)-8s %(message)s', \
                                  datefmt='%m-%d-%Y %H:%M:%S %p')
    handler = logging.StreamHandler(stream=sys.stdout)
    handler.setFormatter(formatter)
    logger = logging.getLogger(name)
    logging.basicConfig(level=os.environ.get("LOGLEVEL", CB_CONFIG['logLevel']))
    logger.addHandler(handler)

    #logging.debug("End setup_custom_logger")

    return logger


def get_adapter_config():
    """Retrieve the runtime configuration for the adapter from a ClearBlade Platform data \
    collection"""
    logging.debug("Begin get_adapter_config")

    logging.debug('Retrieving the adapter configuration from data collection %s', \
        CB_CONFIG['adapterSettingsCollectionName'])

    collection = CB_SYSTEM.Collection(CB_AUTH,
                                      collectionName=CB_CONFIG['adapterSettingsCollectionName'])

    the_query = Query()
    if CB_CONFIG['adapterSettingsItemID'] != "":
        the_query.equalTo("item_id", CB_CONFIG['adapterSettingsItemID'])

    rows = collection.getItems(the_query)

    # Iterate through rows and display them
    for row in rows:
        #TODO - Implement implementation specific logic
        logging.debug(row)

    logging.debug("End get_adapter_config")


#########################
#BEGIN MQTT CALLBACKS
#########################

#TODO - Comment out or remove the MQTT CALLBACKS block if your adapter will not be utilizing MQTT
#TODO - Comment out or remove any of the following MQTT callbacks your adapter implementation does not need

def on_connect(mqtt_client, userdata, flags, result_code):
    """MQTT callback invoked when a connection is established with a broker"""
    logging.debug("Begin on_connect")
    logging.info("Connected to ClearBlade Platform MQTT broker")

    #When the connection to the broker is complete, set up any subscriptions that are needed

    #TODO - If your implementation needs to subscribe to topics, uncomment this block and add your logic here
    # log.Printf("[DEBUG] Begin Configuring Subscription(s)")
    # client.subscribe(CB_CONFIG['adapterTopicRoot'+ '/mySubTopic'])
    global EXIT_APP
    EXIT_APP = True

    logging.debug("End on_connect")

def on_disconnect(mqtt_client, userdata, result_code):
    """MQTT callback invoked when a connection to a broker is lost"""
    logging.debug("Begin on_disconnect")

    if result_code != 0:
        logging.warning("Connection to CB Platform MQTT broker was lost, result code = %s", \
                        result_code)

    #We don't need to worry about manally re-initializing the mqtt client. The auto reconnect
    # logic will automatically try and reconnect. The reconnect interval could be as much as
    # 20 minutes.

    #TODO - Add implementation specific logic here

    logging.debug("End on_disconnect")

def on_subscribe(mqtt_client, userdata, mid, granted_qos):
    """MQTT callback invoked when a subscription has been made"""
    logging.debug("Begin on_subscribe")

    #TODO Add implementation specific logic here 


    logging.debug("End on_subscribe")

def on_unsubscribe(mqtt_client, userdata, mid):
    """MQTT callback invoked when a subscription has been made"""
    logging.debug("Begin on_unsubscribe")

    #TODO Add implementation specific logic here 


    logging.debug("End on_unsubscribe")

def on_publish(mqtt_client, userdata, mid):
    """MQTT callback invoked when a message  has been published"""
    logging.debug("Begin on_publish")

    #TODO Add implementation specific logic here 


    logging.debug("End on_publish")


def on_message(mqtt_client, userdata, mid):
    """MQTT callback invoked when a message is received"""
    logging.debug("Begin on_message")

    #TODO Add implementation specific logic here 


    logging.debug("End on_message")


def on_log(mqtt_client, userdata, level, buf):
    """MQTT callback invoked when MQTT prints a logging statement"""
    logging.debug("Begin on_log")

    logging.debug("End on_log")

#########################
#END MQTT CALLBACKS
#########################


#Main Loop
if __name__ == '__main__':
    #logging.info("Validating command line arguments")
    global EXIT_APP
    EXIT_APP = False

    parse_env_variables(os.environ)
    parse_args(sys.argv)

    LOGGER = setup_custom_logger(ADAPTER_NAME)
    
    check_required_config()

    logging.info("Intializing ClearBlade device client")
    logging.debug("System Key = %s", CB_CONFIG['CB_SYSTEM_KEY'])
    logging.debug("System Secret = %s", CB_CONFIG['CB_SYSTEM_SECRET'])
    logging.debug("HTTP URL = %s", CB_CONFIG['httpURL'] + ":" + CB_CONFIG['httpPort'])

    CB_SYSTEM = System(CB_CONFIG['CB_SYSTEM_KEY'], CB_CONFIG['CB_SYSTEM_SECRET'], CB_CONFIG['httpURL'] + \
                       ":" + CB_CONFIG['httpPort'])



    logging.info("Authenticating to ClearBlade")

    CB_AUTH = None

    if "deviceID" in CB_CONFIG:
        logging.info("Authenticating to ClearBlade with Device ID and Active Key")
        CB_AUTH = CB_SYSTEM.Device(CB_CONFIG['deviceID'], CB_CONFIG['activeKey'])
    elif "CB_SERVICE_ACCOUNT" in CB_CONFIG:
        logging.info("Authenticating to ClearBlade with Device Service Account and Token")
        CB_AUTH = CB_SYSTEM.Device(CB_CONFIG["CB_SERVICE_ACCOUNT"], authToken=CB_CONFIG["CB_SERVICE_ACCOUNT_TOKEN"])
    else:
        logging.error("No device info provided to authenticate to ClearBlade as")
        exit(-1)


    #Retrieve the adapter configuration
    if "adapterSettingsCollectionName" in CB_CONFIG and CB_CONFIG['adapterSettingsCollectionName'] != "":
        logging.info("Retrieving the adapter configuration settings")
        get_adapter_config()

    #########################
    #BEGIN MQTT SPECIFIC CODE
    #########################

    #TODO - Comment out or remove the MQTT SPECIFIC CODE block if your adapter will not be utilizing MQTT

    #Connect to the message broker
    logging.info("Initializing the ClearBlade message broker")
    CB_MQTT = CB_SYSTEM.Messaging(CB_AUTH)

    #TODO - Comment out or remove any of the following 7 lines that you do not need 
    CB_MQTT.on_log = on_log
    CB_MQTT.on_connect = on_connect
    CB_MQTT.on_disconnect = on_disconnect
    CB_MQTT.on_subscribe = on_subscribe
    CB_MQTT.on_unsubscribe = on_unsubscribe
    CB_MQTT.on_publish = on_publish
    CB_MQTT.on_message = on_message

    logging.info("Connecting to the ClearBlade message broker")
    CB_MQTT.connect() #Connect to the msg broker

    #END MQTT SPECIFIC CODE

    while not EXIT_APP:
        try:
            #TODO - Add implementation specific logic here
            pass
        except KeyboardInterrupt:
            EXIT_APP = True
            CB_MQTT.disconnect()
            sys.exit(0)
        except Exception as e:
            logging.info ("EXCEPTION:: %s", str(e))
        #finally:
            #TODO - Add implementation specific logic here
