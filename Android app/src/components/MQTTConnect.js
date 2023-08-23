import React, { useEffect } from 'react'

import { StyleSheet, Text, View } from 'react-native'

import init from 'react_native_mqtt';
import uuid from 'react-native-uuid';
import {AsyncStorage} from "react-native";

const MQTTConnect = ({host, port, options}) => {
    console.log("Host == " + host)
    var mqtt = null;
    var QOS = 0;
    var RETAIN = true;


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

 var n = host

 console.log({n});

var v = 'broker.hivemq.com';
var p = 8000
console.log(v)

this.mqtt = new Paho.MQTT.Client(v, p, clientID);
return mqtt;

const connect = () => {
    if (options) {
        this.QOS = options.qos;
        this.RETAIN = options.retain;
    }

    let currentTime = +new Date();
    let clientID = currentTime + uuid.v1();
    clientID = clientID.slice(0, 23);
    console.log('clientID: ', clientID)

    this.mqtt = new Paho.MQTT.Client(host, port, clientID);
    return mqtt;
    // this.mqtt.onConnectionLost = (message) => {
    //     console.log("Connection Lost ", message)
    //     //this.onMQTTLost(message);
    // };
    // this.mqtt.onMessageArrived = (message) => {
    //     //this.onMQTTMessageArrived(message);
    //     console.log('App onMQTTMessageArrived: ', message);
    //     console.log('App onMQTTMessageArrived payloadString: ', message.payloadString);
    // };
    // this.mqtt.onMessageDelivered = (message) => {
    //     this.onMQTTMessageDelivered(message);
    // };

    // const connectOptions = options ? options : defaultConnectOptions;

    // this.mqtt.connect({
    //     onSuccess: this.onMQTTSuccess,
    //     onFailure: this.onMQTTFailure,
    //     ...connectOptions
    // });
}

}

export default MQTTConnect

