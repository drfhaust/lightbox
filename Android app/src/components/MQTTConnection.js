import init from 'react_native_mqtt';
import uuid from 'react-native-uuid';
import {AsyncStorage} from "react-native";

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

export default class MQTTConnection {
    constructor() {
        this.mqtt = null;
        this.QOS = 0;
        this.RETAIN = true;
    }

    connect(host, port, options = null) {
        if (options) {
            this.QOS = options.qos;
            this.RETAIN = options.retain;
        }

        let currentTime = +new Date();
        let clientID = currentTime + uuid.v1();
        clientID = clientID.slice(0, 23);
        console.log('clientID: ', clientID)

        this.mqtt = new Paho.MQTT.Client(host, port, clientID);
        this.mqtt.onConnectionLost = (message) => {
            console.log("Connection Lost ", message)
            //this.onMQTTLost(message);
        };
        this.mqtt.onMessageArrived = (message) => {
            //this.onMQTTMessageArrived(message);
            console.log('App onMQTTMessageArrived: ', message);
            console.log('App onMQTTMessageArrived payloadString: ', message.payloadString);
        };
        this.mqtt.onMessageDelivered = (message) => {
            this.onMQTTMessageDelivered(message);
        };

        const connectOptions = options ? options : defaultConnectOptions;

        this.mqtt.connect({
            onSuccess: this.onMQTTSuccess,
            onFailure: this.onMQTTFailure,
            ...connectOptions
        });
    }

    onMQTTSuccess = () => {
        console.log("Succesful")
        this.onMQTTConnect()


    }

    onMQTTFailure = (message) => {
       // this.onMQTTLost()
       console.log(message);
        console.log("Unable to connect")
    }

    subscribeChannel(channel) {
        console.log('MQTTConnection subscribeChannel: ', channel)
        if (!this.mqtt || !this.mqtt.isConnected()) {
            console.log("Not Connected")
            return;
        }
        this.mqtt.subscribe(channel, this.QOS);
    }

    unsubscribeChannel(channel) {
        console.log('MQTTConnection unsubscribeChannel: ', channel)
        if (!this.mqtt || !this.mqtt.isConnected()) {
            return;
        }
        this.mqtt.unsubscribe(channel);
    }

    send(channel = null, payload) {
        console.log('MQTTConnection send: ')
        if (!this.mqtt || !this.mqtt.isConnected()) {
            return;
        }

        if (!channel || !payload) {
            return false;
        }
        console.log(`MQTTConnection send publish channel: ${channel}, payload: ${payload} qos: ${this.QOS} retained: ${this.RETAIN}`)
        this.mqtt.publish(channel, payload, this.QOS, this.RETAIN);
    }

    close() {
        this.mqtt && this.mqtt.disconnect();
        this.mqtt = null;
    }
    

}

MQTTConnection.prototype.onMQTTConnect = null
MQTTConnection.prototype.onMQTTLost = null
MQTTConnection.prototype.onMQTTMessageArrived = null
MQTTConnection.prototype.onMQTTMessageDelivered = null