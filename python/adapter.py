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

def parse_args(argv):
    """Parse the command line arguments"""

    parser = argparse.ArgumentParser(description='Start ClearBlade Adapter')
    parser.add_argument('--systemKey', required=True, help='The System Key of the ClearBlade \
                        Plaform "System" the adapter will connect to.')

    parser.add_argument('--systemSecret', required=True, help='The System Secret of the \
                        ClearBlade Plaform "System" the adapter will connect to.')

    parser.add_argument('--deviceID', required=True, \
                        help='The id/name of the device that will be used for device \
                        authentication against the ClearBlade Platform or Edge, defined \
                        within the devices table of the ClearBlade platform.')

    parser.add_argument('--activeKey', required=True, \
                        help='The active key of the device that will be used for device \
                        authentication against the ClearBlade Platform or Edge, defined within \
                        the devices table of the ClearBlade platform.')

    parser.add_argument('--httpUrl', dest="httpURL", default="http://localhost", \
                        help='The HTTP URL of the ClearBlade Platform or Edge the adapter will \
                        connect to. The default is https://localhost.')

    parser.add_argument('--httpPort', dest="httpPort", default="9000", \
                        help='The HTTP Port of the ClearBlade Platform or Edge the adapter will \
                        connect to. The default is 9000.')

    parser.add_argument('--messagingUrl', dest="messagingURL", default="localhost", \
                        help='The MQTT URL of the ClearBlade Platform or Edge the adapter will \
                        connect to. The default is https://localhost.')

    parser.add_argument('--messagingPort', dest="messagingPort", default="1883", \
                        help='The MQTT Port of the ClearBlade Platform or Edge the adapter will \
                        connect to. The default is 9000.')

    parser.add_argument('--adapterSettingsCollection', dest="adapterSettingsCollectionName", \
                        default="", \
                        help='The name of the ClearBlade Platform data collection which contains \
                        runtime configuration settings for the adapter. The default is "".')

    parser.add_argument('--adapterSettingsItem', dest="adapterSettingsItemID", default="", \
                        help='The "item_id" of the row, within the ClearBlade Platform data \
                        collection which contains runtime configuration settings, that should \
                        be used to configure the adapter. The default is "".')

    parser.add_argument('--topicRoot', dest="adapterTopicRoot", default="", \
                        help='The root of MQTT topics this adapter will subscribe and publish to. \
                        The default is "".')

    parser.add_argument('--deviceProvisionSvc', dest="deviceProvisionSvc", default="", \
                        help='The name of a service that can be invoked to provision IoT devices \
                        within the ClearBlade Platform or Edge. The default is "".')

    parser.add_argument('--deviceHealthSvc', dest="deviceHealthSvc", default="", \
                        help='The name of a service that can be invoked to provide the health of \
                        an IoT device to the ClearBlade Platform or Edge. The default is "".')

    parser.add_argument('--deviceLogsSvc', dest="deviceLogsSvc", default="", \
                        help='The name of a service that can be invoked to provide IoT device \
                        logging information to the ClearBlade Platform or Edge. The default is "".')

    parser.add_argument('--deviceStatusSvc', dest="deviceStatusSvc", default="", \
                        help='The name of a service that can be invoked to provide the status of \
                        an IoT device to the ClearBlade Platform or Edge. The default is "".')

    parser.add_argument('--deviceDecommissionSvc', dest="deviceDecommissionSvc", default="", \
                        help='The name of a service that can be invoked to decommission IoT \
                        devices within the ClearBlade Platform or Edge. The default is "".')

    parser.add_argument('--logLevel', dest="logLevel", default="INFO", choices=['CRITICAL', \
                        'ERROR', 'WARNING', 'INFO', 'DEBUG'], help='The level of logging that \
                        should be utilized by the adapter. The default is "INFO".')

    parser.add_argument('--logCB', dest="logCB", default=False, action='store_true',\
                        help='Flag presence indicates logging information should be printed for \
                        ClearBlade libraries.')

    parser.add_argument('--logMQTT', dest="logMQTT", default=False, action='store_true',\
                        help='Flag presence indicates MQTT logs should be printed.')

    #TODO Add implementation specific command line arguments here


    return vars(parser.parse_args(args=argv[1:]))


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

    CB_CONFIG = parse_args(sys.argv)
    LOGGER = setup_custom_logger(ADAPTER_NAME)

    if not CB_CONFIG['logCB']:
        logging.debug("Setting cbLogs.DEBUG to False")
        cbLogs.DEBUG = False

    if not CB_CONFIG['logMQTT']:
        logging.debug("Setting cbLogs.MQTT_DEBUG to False")
        cbLogs.MQTT_DEBUG = False

    logging.info("Intializing ClearBlade device client")
    logging.debug("System Key = %s", CB_CONFIG['systemKey'])
    logging.debug("System Secret = %s", CB_CONFIG['systemSecret'])
    logging.debug("HTTP URL = %s", CB_CONFIG['httpURL'] + ":" + CB_CONFIG['httpPort'])

    CB_SYSTEM = System(CB_CONFIG['systemKey'], CB_CONFIG['systemSecret'], CB_CONFIG['httpURL'] + \
                       ":" + CB_CONFIG['httpPort'])

    logging.info("Authenticating to ClearBlade")
    logging.debug("Device ID = %s", CB_CONFIG['deviceID'])
    logging.debug("Device Active Key = %s", CB_CONFIG['activeKey'])

    CB_AUTH = CB_SYSTEM.Device(CB_CONFIG['deviceID'], CB_CONFIG['activeKey'])

    #Retrieve the adapter configuration
    if CB_CONFIG['adapterSettingsCollectionName'] != "":
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
