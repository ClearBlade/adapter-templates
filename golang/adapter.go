package main

import (
	"flag"
	"log"
	"os"
	"strings"
	"time"

	lumberjack "gopkg.in/natefinch/lumberjack.v2"

	cb "github.com/clearblade/Go-SDK"
	MQTT "github.com/eclipse/paho.mqtt.golang"
	"github.com/hashicorp/logutils"
)

var (
	httpURL         string //Variable to hold the HTTP URL of the ClearBlade Platform or Edge the adapter will connect to
	httpPort        string //Variable to hold the HTTP Port of the ClearBlade Platform or Edge the adapter will connect to
	messagingURL    string //Variable to hold the MQTT URL of the ClearBlade Platform or Edge the adapter will connect to
	messagingPort   string //Variable to hold the MQTT Port of the ClearBlade Platform or Edge the adapter will connect to
	sysKey          string //Variable to hold the System Key of the ClearBlade Plaform "System" the adapter will connect to
	sysSec          string //Variable to hold the System Secret of the ClearBlade Plaform "System" the adapter will connect to
	deviceID        string //Variable to hold the id/name of the device that will be used for device authentication against the ClearBlade Platform or Edge
	deviceActiveKey string //Variable to hold the active key of the device that will be used for device authentication against the ClearBlade Platform or Edge

	adapterSettingsCollectionName string //Variable to hold the name of the ClearBlade Platform data collection which contains runtime configuration settings for the adapter
	adapterSettingsItemID         string //Variable to hold the "item_id" of the row, within the ClearBlade Platform data collection which contains runtime configuration settings, that should be used to configure the adapter

	adapterTopicRoot string //Variable to hold the root of MQTT topics this adapter will subscribe and publish to

	deviceProvisionSvc    string //Variable to hold the name of a service that can be invoked to provision IoT devices within the ClearBlade Platform or Edge
	deviceHealthSvc       string //Variable to hold the name of a service that can be invoked to provide the health of an IoT device to the ClearBlade Platform or Edge
	deviceLogsSvc         string //Variable to hold the name of a service that can be invoked to provide IoT device logging information to the ClearBlade Platform or Edge
	deviceStatusSvc       string //Variable to hold the name of a service that can be invoked to provide the status of an IoT device to the ClearBlade Platform or Edge
	deviceDecommissionSvc string //Variable to hold the name of a service that can be invoked to decommission IoT devices within the ClearBlade Platform or Edge

	logLevel string
	logMQTT  bool

	deviceClient *cb.DeviceClient
)

const (
	platURL  = "http://localhost"
	platPort = "9000"
	messURL  = "localhost"
	messPort = "1883"
)

func init() {
	flag.StringVar(&sysKey, "systemKey", "", "system key (required)")
	flag.StringVar(&sysSec, "systemSecret", "", "system secret (required)")
	flag.StringVar(&deviceID, "deviceID", "", "id/name of device used for device authentication (required)")
	flag.StringVar(&deviceActiveKey, "deviceActiveKey", "", "active key of the device used for device authentication (required)")

	flag.StringVar(&httpURL, "httpURL", platURL, "ClearBlade Platform or ClearBlade Edge HTTP URL. Will default to \"http://localhost\". (optional)")
	flag.StringVar(&httpPort, "httpPort", platPort, "ClearBlade Platform or ClearBlade Edge http port. Will default to \"9000\". (optional)")

	flag.StringVar(&messagingURL, "messagingURL", messURL, "ClearBlade Platform or ClearBlade Edge messaging URL. Will default to \"localhost\". (optional)")
	flag.StringVar(&messagingPort, "messagingPort", messPort, "ClearBlade Platform or ClearBlade Edge messaging URL. Will default to \"1883\". (optional)")

	flag.StringVar(&adapterSettingsCollectionName, "adapterSettingsCollection", "", "The name of a data collection containing runtime settings for the adapter (optional)")
	flag.StringVar(&adapterSettingsItemID, "adapterSettingsItem", "", "The \"item_id\" of a specific row within the adapter runtime settings data collection to retrieve (optional)")

	flag.StringVar(&adapterTopicRoot, "topicRoot", "", "The root MQTT topic the adapter should use when publishing or subscribing. (optional)")

	flag.StringVar(&deviceProvisionSvc, "deviceProvisionSvc", "", "The name of a ClearBlade Platform code service that will receive device provisioning requests. (optional)")
	flag.StringVar(&deviceHealthSvc, "deviceHealthSvc", "", "The name of a ClearBlade Platform code service that will receive device health requests. (optional)")
	flag.StringVar(&deviceLogsSvc, "deviceLogsSvc", "", "The name of a ClearBlade Platform code service that will receive device log requests. (optional)")
	flag.StringVar(&deviceStatusSvc, "deviceStatusSvc", "", "The name of a ClearBlade Platform code service that will receive device status requests. (optional)")
	flag.StringVar(&deviceDecommissionSvc, "deviceDecommissionSvc", "", "The name of a ClearBlade Platform code service that will receive device decommissioning requests. (optional)")

	flag.StringVar(&logLevel, "logLevel", "warn", "The level of logging to use. Available levels are 'debug', 'warn', 'error' (optional)")
	flag.BoolVar(&logMQTT, "logMQTT", false, "Indicates MQTT log information should be written")

	//TODO - Add code to handle any other command line arguments specific to your implmentation
}

func usage() {
	log.Printf("Usage: adapter [options]\n\n")
	flag.PrintDefaults()
}

func validateFlags() {
	log.Printf("Validating command line options")

	flag.Parse()

	//TODO If any implmentation specific REQUIRED command line parameters have been added,
	//add the associated variables to this if statement
	if sysKey == "" ||
		sysSec == "" ||
		deviceID == "" ||
		deviceActiveKey == "" {

		log.Printf("[ERROR] Missing required flags\n\n")
		flag.Usage()
		os.Exit(1)
	}

	if logLevel != "error" && logLevel != "warn" && logLevel != "debug" {
		log.Printf("[ERROR] Invalid log level specified\n\n")
		flag.Usage()
		os.Exit(1)
	}

	//TODO Add any other implementation specific command line argument validation logic here
}

//ClearBlade Device Client init helper
func initCbDeviceClient() {
	log.Printf("Initializing ClearBlade device client")

	log.Printf("[DEBUG] Setting System Key to %s", sysKey)
	log.Printf("[DEBUG] Setting System Secret to %s", sysSec)
	log.Printf("[DEBUG] Setting device ID to %s", deviceID)
	log.Printf("[DEBUG] Setting active key to %s", deviceActiveKey)
	log.Printf("[DEBUG] Setting platform URL to %s", httpURL+":"+httpPort)
	log.Printf("[DEBUG] Setting messaging URL to %s", messagingURL+":"+messagingPort)

	deviceClient = cb.NewDeviceClientWithAddrs(httpURL+":"+httpPort, messagingURL+":"+messagingPort, sysKey, sysSec, deviceID, deviceActiveKey)

	for err := deviceClient.Authenticate(); err != nil; {
		log.Printf("[WARN] Error authenticating to platform: %s", err.Error())

		//TODO Add logic to differentiate between timeout errors and bad username/password errors
		//Only retry authentication if a timeout or connection issue occurs

		log.Printf("[WARN] Will retry in 1 minute...")

		// sleep 1 minute
		time.Sleep(time.Duration(time.Minute * 1))
		err = deviceClient.Authenticate()
	}

	log.Printf("[DEBUG] Initializing MQTT with callbacks")
	var callbacks = &cb.Callbacks{OnConnectionLostCallback: OnConnectLost, OnConnectCallback: OnConnect}

	//TODO - replace "myAdapter" with the name of your adapter
	if mqtterr := deviceClient.InitializeMQTTWithCallback("myAdapter"+deviceID, "", 30, nil, nil, callbacks); mqtterr != nil {
		log.Fatalf("[ERROR] initCbClient: Unable to initialize MQTT connection to broker %s: %s",
			messagingURL+":"+messagingPort, mqtterr.Error())
	}

	log.Printf("Finished initializing ClearBlade device client")
}

//getAdapterConfig - Retrieve adapter runtime configuration parameters from a platform/edge data collection
func getAdapterConfig() error {
	log.Printf("Retrieving adapter runtime settings")

	query := &cb.Query{}

	if adapterSettingsItemID != "" {
		log.Printf("[DEBUG] Adapter settings item id = %s", adapterSettingsItemID)
		query := cb.NewQuery()
		query.EqualTo("item_id", adapterSettingsItemID)
	}

	//Retrieve the adapter configuration row. Passing a nil query results in all rows being returned
	results, err := deviceClient.GetDataByName(adapterSettingsCollectionName, query)
	if err != nil {
		log.Printf("[WARN] Adapter configuration could not be retrieved. Error: %s", err.Error())
		return err
	}

	log.Printf("[DEBUG] Adapter settings retrieved: %#v", results)

	//TODO - Add implemenation specific logic
	//example: myVar = results["DATA"].([]interface{})[0].(map[string]interface{})["myCollectionColumn"].(string)

	log.Printf("Finished retrieving adapter runtime settings")

	return nil
}

//OnConnectLost - MQTT callback invoked when a connection to a broker is lost
func OnConnectLost(client MQTT.Client, connerr error) {
	log.Printf("[WARN] Connection to broker was lost: %s", connerr.Error())

	//We don't need to worry about manally re-initializing the mqtt client. The auto reconnect logic will
	//automatically try and reconnect. The reconnect interval could be as much as 20 minutes.

	//TODO - Add any implementation logic here
}

//OnConnect - MQTT callback invoked when a connection is established with a broker
//When the connection to the broker is complete, set up the subscriptions
func OnConnect(client MQTT.Client) {
	log.Printf("Connected to ClearBlade Platform MQTT broker")

	//TODO - If your implementation needs to subscribe to topics, uncomment this block and add your logic here
	// log.Printf("[DEBUG] Begin Configuring Subscription(s)")

	// for mySubscribeChannel, err = deviceClient.Subscribe(adapterTopicRoot+"/"+subscribeTopic, messagingQos); err != nil; {
	// 	log.Printf("[WARN] Error subscribing to topics: %s", err.Error())

	// 	//Wait 30 seconds and retry
	// 	log.Printf("[DEBUG] Waiting 30 seconds to retry subscriptions")
	// 	time.Sleep(time.Duration(30 * time.Second))
	// 	mySubscribeChannel, err = deviceClient.Subscribe(adapterTopicRoot+"/"+subscribeTopic, messagingQos)
	// }
}

func main() {
	flag.Usage = usage
	validateFlags()

	//////////////////////////////////////////
	//BEGIN Custom logfile implemenation block
	//////////////////////////////////////////

	// If you do not wish to implement rolling log files, comment out/remove this block of code

	//TODO - Rename the file "/var/log/adapter.log" to provide a name that describes the adapter
	logFileName := "/var/log/adapter.log"

	_, err := os.OpenFile(logFileName, os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0666)

	if err != nil {
		log.Fatalf("error opening file: %s", err.Error())
	}

	log.SetFlags(log.LstdFlags | log.Lshortfile)

	//TODO - Modify these values to fit the needs of your adapter
	var lumberjackLogger = &lumberjack.Logger{
		Filename:   logFileName,
		MaxSize:    10, // megabytes
		MaxBackups: 5,
		MaxAge:     28, //days
	}

	filter := &logutils.LevelFilter{
		Levels:   []logutils.LogLevel{"DEBUG", "WARN", "ERROR"},
		MinLevel: logutils.LogLevel(strings.ToUpper(logLevel)),
		Writer:   lumberjackLogger,
	}
	log.Printf("Logging output is being redirected to %s. View %s for adapter log details.", logFileName, logFileName)
	log.SetOutput(filter)

	//////////////////////////////////////////
	//END Custom logfile implemenation block
	//////////////////////////////////////////

	///////////////////////////
	//BEGIN MQTT log file block
	///////////////////////////

	//Initialize MQTT Logging
	if logMQTT {
		//Always show critical log messages
		MQTT.CRITICAL = log.New(lumberjackLogger, "CRITICAL: ", log.Ldate|log.Ltime|log.Lshortfile)
		if logLevel == "debug" {
			MQTT.DEBUG = log.New(lumberjackLogger, "DEBUG: ", log.Ldate|log.Ltime|log.Lshortfile)
		}
		if logLevel == "warn" || logLevel == "debug" {
			MQTT.WARN = log.New(lumberjackLogger, "WARN: ", log.Ldate|log.Ltime|log.Lshortfile)
		}
		if logLevel == "error" || logLevel == "warn" || logLevel == "debug" {
			MQTT.ERROR = log.New(lumberjackLogger, "ERROR: ", log.Ldate|log.Ltime|log.Lshortfile)
		}
	}

	///////////////////////////
	//END MQTT log file block
	///////////////////////////

	//Initialize CB device client
	initCbDeviceClient()

	if adapterSettingsCollectionName != "" {
		//Retrieve adapter config info
		getAdapterConfig()
	}

	//TODO - Add your implemenation specific code here
	//At this point, the CB device client will have been
	//initialized and authenticated to the ClearBlade Platform or ClearBlade Edge
	select {}
}
