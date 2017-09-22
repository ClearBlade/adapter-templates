# ADAPTER-SPEC

When creating adapters, there are tasks that adapters must accomplished in order for the adapter to communicate with the ClearBlade Platform or ClearBlade Edge. This repository contains ClearBlade adapter __template__ programs, written in various languages, that contain the logic needed to implement these commonly needed tasks. 


The adapter template programs currently supports the following:
  1. Command line argument definition and validation
  2. Authenticating to the ClearBlade Platform or ClearBlade Edge as a device
  3. Retrieving adapter runtime configuration parameters from a ClearBlade Platform data collection
  4. Connecting to a ClearBlade Platform or ClearBlade Edge MQTT message broker with language specific MQTT callbacks
  5. Custom logging with log level filtering

## Setup
---
Setting up any adapter dependencies are specific to the language in which the adapter was written. See the README.md file within each language implementation for further details.

## Todo
---
 - Add adapter templates for other languages (java, c, c++)
