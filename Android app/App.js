/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 *
 * @format
 * @flow strict-local
 */

import React, { useEffect, useState } from 'react';
import {
  SafeAreaView,
  ScrollView,
  StatusBar,
  StyleSheet,
  Text,
  useColorScheme,
  View,
  Image,
  Dimensions,
  TouchableOpacity,
} from 'react-native';

import Logo from './src/assets/logo.png';

import init from 'react_native_mqtt';
import uuid from 'react-native-uuid';
import { AsyncStorage } from "react-native";

import { Buffer } from 'buffer';
global.Buffer = Buffer;

import { Colors } from 'react-native/Libraries/NewAppScreen';

const App = () => {
  const isDarkMode = useColorScheme() === 'dark';
  const backgroundStyle = {
    backgroundColor: isDarkMode ? Colors.darker : Colors.lighter,
  };

  var mqtt = null;
  var QOS = 0;
  var RETAIN = true;
  var options = null;

  init({
    size: 10000,
    storageBackend: AsyncStorage,
    defaultExpires: 1000 * 3600 * 24,
    enableCache: true,
    sync: {},
  });

  const defaultConnectOptions = {
    reconnect: false,
    cleanSession: true,
    mqttVersion: 3,
    keepAliveInterval: 60,
    timeout: 60
  }

  if (options) {
    this.QOS = options.qos;
    this.RETAIN = options.retain;
  }

  let currentTime = +new Date();
  let clientID = currentTime + uuid.v1();
  clientID = clientID.slice(0, 23);
  console.log('clientID: ', clientID)

  var host = 'broker.hivemq.com';
  var port = 8000

  mqtt = new Paho.MQTT.Client(host, port, clientID);

  const connectOptions = options ? options : defaultConnectOptions;

  const onMQTTSuccess = () => {
    console.log("Succesful")
    subscribeChannel("dan/in")
  }


  const subscribeChannel = (channel) => {
    console.log('MQTTConnection subscribeChannel: ', channel)
    if (!mqtt || !mqtt.isConnected()) {
      console.log("Not Connected")
      return;
    }
    mqtt.subscribe(channel, QOS);
  }


  const onMQTTFailure = (message) => {
    // this.onMQTTLost()
    console.log(message);
    console.log("Unable to connect")
  }

  mqtt.connect({
    onSuccess: onMQTTSuccess,
    onFailure: onMQTTFailure,
    ...connectOptions
  });

  const roundUp = (num) => { return Math.round((parseFloat(num) + Number.EPSILON) * 100) / 100 }


  useEffect(() => {
    mqtt.onConnectionLost = (message) => {
      console.log("Connection Lost ", message)
      
      //this.onMQTTLost(message);
    };
    mqtt.onMessageArrived = (message) => {
      //this.onMQTTMessageArrived(message);
      console.log('App onMQTTMessageArrived: ', message);
      console.log('App onMQTTMessageArrived payloadString: ', message.payloadString);
      //var value = message.payloadString.replace(/\\/g,'');
      var _main = roundUp(JSON.parse(JSON.parse(message.payloadString).Main))
      var _inv = roundUp(JSON.parse(JSON.parse(message.payloadString).Inv))
      var _gen = roundUp(JSON.parse(JSON.parse(message.payloadString).Gen))
      var _curr = roundUp(JSON.parse(JSON.parse(message.payloadString).Curr))
      var _mode = roundUp(JSON.parse(JSON.parse(message.payloadString).Mode))
      var _batv = roundUp(JSON.parse(JSON.parse(message.payloadString).BatV))
      var _genv = roundUp(JSON.parse(JSON.parse(message.payloadString).GenV))
      var _opmode = roundUp(JSON.parse(JSON.parse(message.payloadString).OpMode))

      value = {
        "Main": _main,
        "Inv": _inv,
        "Gen": _gen,
        "Curr": _curr,
        "Mode": _mode,
        "OpMode": _opmode,
        "BatV": _batv,
        "GenV": _genv,
      }
      console.log({ value })
      setPayLoad(value)
    };
    mqtt.onMessageDelivered = (message) => {
      //this.onMQTTMessageDelivered(message);
      console.log("Message Sent ", message)
    };
  }, [])

  const [payload, setPayLoad] = useState({
    "Main": 0,
    "Inv": 0,
    "Gen": 0,
    "Curr": 0,
    "Mode": 0,
    "OpMode": 200,
    "BatV": 0,
    "GenV": 0,
  })

  const PublishMessage = (topic, message) => {
    console.log('MQTTConnection send: ' + message)
    if (!mqtt || !mqtt.isConnected()) {
      console.log("Not connected")
      return;
    }

    if (!topic || !message) {
      console.log("message or topic is empty")
      return false;
    }
    console.log(`MQTTConnection send publish channel: ${topic}, payload: ${message} qos: ${QOS} retained: ${RETAIN}`)
    mqtt.publish(topic, message, QOS, RETAIN);
  }

  const getWatt = () => {
    if (payload.Mode == 1) {
      return roundUp(payload.Main * payload.Curr)
    }
    else if (payload.Mode == 2) {
      return roundUp(payload.Inv * payload.Curr)
    }
    else if (payload.Mode == 3) {
      return roundUp(payload.Gen * payload.Curr)
    }
    else if (payload.Mode == 4) {
      return roundUp(0)
    }
    else if (payload.Mode == 5) {
      return roundUp(0)
    }
  }

  const [autoManualToggle, setAutoManualToggle] = useState(() =>{
    return false;
  })
  const [activeButtonAction, setActiveButtonAction] = useState(0);



  return (

    <SafeAreaView style={(backgroundStyle, styles.screenContainer)}>
      <StatusBar barStyle={isDarkMode ? 'light-content' : 'dark-content'} />
      <ScrollView
        contentInsetAdjustmentBehavior="automatic"
        style={(backgroundStyle, styles.innerScreeenContainer)}
        contentContainerStyle={{
          height: '100%',
        }}>
        {/* <View style={styles.nameCon}></View> */}
        <View style={styles.menuCon}>
          <View style={styles.labelCon}>
            <View style={payload.Mode == 1 ? styles.activeLabel : styles.label}>
              <Text style={styles.labelText}>Main</Text>
            </View>
            <View style={payload.Mode == 2 ? styles.activeLabel : styles.label}>
              <Text style={styles.labelText}>Gen</Text>
            </View>
            <View style={payload.Mode == 3 ? styles.activeLabel : styles.label}>
              <Text style={styles.labelText}>Inv</Text>
            </View>
          </View>
          <View style={styles.valueCon}>
            <Text style={styles.value}>{payload["Main"]}V</Text>
            <Text style={styles.value}>{payload["Gen"]}V</Text>
            <Text style={styles.value}>{payload["Inv"]}V</Text>
          </View>
        </View>
        <View style={styles.bigDisplayCon}>
          <Text style={styles.bigDisplayText}>Out: {getWatt()}w</Text>
        </View>
        <View style={styles.menuCon}>
          <View style={styles.labelCon}>
            <View style={styles.label}>
              <Text style={styles.labelText}>MODE</Text>
            </View>
            <View style={styles.label}>
              <Text style={styles.labelText}>Bat V</Text>
            </View>
            <View style={styles.label}>
              <Text style={styles.labelText}>Gen V</Text>
            </View>
          </View>
          <View style={styles.valueCon}>
            <Text style={styles.value}>{payload.OpMode == 1 ? "Auto" : payload.OpMode == 2 ? "Manual" : ""}</Text>
            <Text style={styles.value}>{payload.BatV}%</Text>
            <Text style={styles.value}>{payload.GenV}%</Text>
          </View>
        </View>
        <TouchableOpacity onPress={() => {

          setAutoManualToggle(()=>{
            if (autoManualToggle === true){
              return false;
            }
            else{
              return true;
            }
          })
          if (autoManualToggle === true){
            var value = 0;
          }
          else{
            var value = 1
          }
          //PublishMessage("dan/out", "{phmode: " + (value).toString() + "}")
          PublishMessage("dan/out", "{phout: " + (activeButtonAction).toString() + " , pshtdwn: 0, phmode: " + (value).toString() + "}")
        }}>
          <View style={[styles.bigButton, autoManualToggle === true ? { backgroundColor: "#ff0000" } : { backgroundColor: '#12B293' }]}>
            <Text style={styles.buttonText}>{autoManualToggle === true ? "Auto" : "Manual"}</Text>
          </View>
        </TouchableOpacity>

        <View style={styles.smallButtonCon}>
          <TouchableOpacity style={[styles.smallButton, activeButtonAction == 1 ? { backgroundColor: "#ff0000" } : { backgroundColor: '#12B293' }]} onPress={() => {
            // PublishMessage("dan/out", "{phout: 1}")
            // PublishMessage("dan/out", "{pshtdwn: 0}")
            if (autoManualToggle === true) {
              var v = 1
            }
            else {
              var v = 0;
            }
            PublishMessage("dan/out", "{phout: 1, pshtdwn: 0, phmode: " + (v).toString() + "}")
            setActiveButtonAction(1);

          }}>
            <View >
              <Text style={styles.buttonText}>Main</Text>
            </View>
          </TouchableOpacity>

          <TouchableOpacity style={[styles.smallButton, activeButtonAction == 2 ? { backgroundColor: "#ff0000" } : { backgroundColor: '#12B293' }]} onPress={() => {
            //PublishMessage("dan/out", "{phout: 2}")
            //PublishMessage("dan/out", "{pshtdwn: 0}")
            if (autoManualToggle === true) {
              var v = 1
            }
            else {
              var v = 0;
            }
            PublishMessage("dan/out", "{phout: 2, pshtdwn: 0, phmode: " + (v).toString() + "}")
            setActiveButtonAction(2);
          }}>
            <View >
              <Text style={styles.buttonText}>Inv</Text>
            </View>
          </TouchableOpacity>

        </View>
        <View style={styles.smallButtonCon}>
          <TouchableOpacity style={[styles.smallButton, activeButtonAction == 3 ? { backgroundColor: "#ff0000" } : { backgroundColor: '#12B293' }]} onPress={() => {
            if (autoManualToggle === true) {
              var v = 1
            }
            else {
              var v = 0;
            }
            PublishMessage("dan/out", "{phout: 3, pshtdwn: 0, phmode: " + (v).toString() + "}")
            //PublishMessage("dan/out", "{phout: 3}")
            // PublishMessage("dan/out", "{pshtdwn: 0}")
            setActiveButtonAction(3);
          }}>
            <View >
              <Text style={styles.buttonText}>Gen</Text>
            </View>
          </TouchableOpacity>

          <TouchableOpacity style={[styles.smallButton, activeButtonAction == 4 ? { backgroundColor: "#ff0000" } : { backgroundColor: '#12B293' }]} onPress={() => {
            if (autoManualToggle === true) {
              var v = 1
            }
            else {
              var v = 0;
            }
            PublishMessage("dan/out", "{phout: 4, pshtdwn: 0, phmode: " + (v).toString() + "}")
            //PublishMessage("dan/out", "{pshtdwn: 0}")
            setActiveButtonAction(4);
          }}>
            <View>
              <Text style={styles.buttonText}>Idle</Text>
            </View>
          </TouchableOpacity>

        </View>
        <TouchableOpacity onPress={() => {
          //PublishMessage("dan/out", "{pshtdwn: 1}")
          if (autoManualToggle === true) {
            var v = 1
          }
          else {
            var v = 0;
          }
          PublishMessage("dan/out", "{phout: 4, pshtdwn: 1, phmode: " + (v).toString() + "}")
          setActiveButtonAction(0);
        }}>
          <View style={styles.bigButton}>
            <Text style={styles.buttonText}>Shutdown</Text>
          </View>
        </TouchableOpacity>

      </ScrollView>
    </SafeAreaView>
    // <Text style={styles.test}>Hey!</Text>
  );
};

const styles = StyleSheet.create({
  screenContainer: {
    flex: 1,
    backgroundColor: '#000000',
  },
  innerScreeenContainer: {
    width: '100%',
    paddingHorizontal: 15,
    paddingTop: 10,
  },
  innerContainer: {
    width: '100%',
    flex: 1,
    height: '100%',
    flexDirection: 'column',
    alignItems: 'center',
    justifyContent: 'center',
  },
  nameCon: {
    height: 50,
    width: '100%',
  },
  menuCon: {
    height: 100,
    width: '100%',
    marginBottom: 30,
  },
  labelCon: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    marginBottom: 5
  },
  valueCon: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-around',
  },
  label: {
    height: 40,
    width: '30%',
    borderRadius: 5,
    marginTop: Dimensions.get("window").height * 0.03,
    backgroundColor: '#12B293',
    alignItems: 'center',
    justifyContent: 'center',
  },
  activeLabel: {
    height: 40,
    width: '30%',
    borderRadius: 5,
    marginTop: Dimensions.get("window").height * 0.03,
    backgroundColor: '#ff0000',
    alignItems: 'center',
    justifyContent: 'center',
  },
  labelText: {
    color: '#fff',
    fontWeight: '600',
    fontSize: 20,
  },
  value: {
    fontSize: 25,
    color: '#4AD395',
    fontWeight: '600'
  },

  activeValue: {
    fontSize: 25,
    color: '#ff0000',
    fontWeight: '600'
  },
  bigDisplayCon: {
    height: 100,
    width: '100%',
    borderRadius: 10,
    backgroundColor: '#111111',
    alignItems: 'center',
    justifyContent: 'center',
    marginBottom: 30,
  },
  bigDisplayText: {
    fontSize: 50,
    fontWeight: '600',
    color: '#12B293',
  },
  bigButton: {
    width: '100%',
    height: 50,
    backgroundColor: '#12B293',
    marginBottom: 30,
    borderRadius: 5,
    alignItems: 'center',
    justifyContent: 'center',
  },
  buttonText: {
    fontWeight: '600',
    fontSize: 20,
    color: '#fff',
  },
  smallButtonCon: {
    width: '100%',
    flexDirection: 'row',
    alignContent: 'center',
    justifyContent: 'space-between',
    marginBottom: 30,
  },
  smallButton: {
    backgroundColor: '#12B293',
    width: '45%',
    height: 50,
    borderRadius: 5,
    alignItems: 'center',
    justifyContent: 'center',
  },
});

export default App;
