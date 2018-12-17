#############################################
#
# MQTT subscriber with SQLITE storage
#
#
import time
import sqlite3
import paho.mqtt.client as paho
from MQTT_Errors import *
import logging

#############################################
#
# config
#

broker="192.168.1.44"   # PINAT
storeDBfile="mydb"
logFile="mqtt.log"
topic="#"
clientID="client-003"

#############################################
#
# logging output goes ONLY to the log file
logging.basicConfig(filename=logFile,format='%(asctime)s %(message)s', level=logging.DEBUG)


############################################
#
# global variables
#
client=None

############################################
#
# shut everything down

def terminate():
    global client

    logging.info("Terminating")
    if client:
        client.disconnect()
        client.loop_stop()

    # terminate the program
    exit()

############################################
#
# MQTT callbacks
#
############################################

def on_connect(client,userdata,flags,rc):
    if rc==0:
        logging.info("on_connect: Connection successful")
        return

    # anything else is a connection refusal

    logMsg="Connection refused - {}"
    if rc==1:
        logging.info(logMsg.format("incorrect protocol version"))
    elif rc==2:
        logging.info(logMsg.format("invalid client identifier"))
    elif rc==3:
        logging.info(logMsg.format("server unavailable"))
    elif rc==4:
        logging.info(logMsg.format("bad username or password"))
    elif rc==5:
        logging.info(logMsg.format("not authorised"))
    else:
        logging.info("Unknown cnnection result code "+str(rc))

    # we are not connected to a broker so terminate the program
    terminate()


def on_message(client, userdata, message):

    print "on_message callback"
    timestamp=time.time()
    logging.info("timestamp="+str(timestamp))
    logging.info("message received "+ str(message.payload.decode("utf-8")))
    logging.info("message topic="+ message.topic)
    logging.info("message qos="+ str(message.qos))
    logging.info("message retain flag="+ str(message.retain))
    # here we would put the message into a database
    # we cannot use the global db object in this thread
    try:
        db = sqlite3.connect(storeDBfile)
        cursor=db.cursor()
        cursor.execute('''INSERT INTO MQTT(topic,timestamp,payload, qos)
                  VALUES(?,?,?,?)''', (message.topic,timestamp, message.payload.decode("utf-8"), message.qos))
        db.commit()
        db.close()
    except Exception as e:
        logging.info("INSERT into MQTT failed err="+e.message)
        terminate()

    logging.info("Added message to database.")

def on_subscribe(client, userdata, mid, qos):
    # just log the fact that the subscription took place
    logging.info("on subscribe callback")


def on_log(client,userdata,level,buf):
    print "PAHO log:",buf

#########################################################################
#
# main
#
#
#
#########################################################################
def main():

    global client, clientID, storeDBfile, broker
    ##############################################
    #
    # connect to the database - will create a file if it doesn't exist
    #
    try:
        db=sqlite3.connect(storeDBfile)
        try:
            cursor = db.cursor()
            cursor.execute('''CREATE TABLE MQTT(Topic TEXT, timestamp TEXT, payload TEXT, qos INTEGER)''')
            db.commit()
            logging.info("SQLITE MQTT table created")
        except:
            logging.info("SQLITE MQTT table exists")
        finally:
            # the database has to be re-opened in the on_message callback
            db.close()
    except:
        logging.info("Unable to connect to database ["+storeDBfile+"]")
        terminate()

    ################################################
    #
    # create the paho MQTT client, set the callbacks
    # and start the client loop
    #

    try:
        client=paho.Client(clientID)

        client.connect(broker)  # connect

        # now setup MQTT callbacks
        client.on_message = on_message
        client.on_connect = on_connect
        client.on_subscribe = on_subscribe
        client.on_log = on_log
        client.loop_start()  # start loop to process received messages

    except Exception as e:
        logging.critical("unable to create paho client or connect. Err="+e.message)
        terminate()


    #############################################
    #
    # get started
    #
    logging.info("connecting to broker at "+broker)

    (result,mid)=client.subscribe(topic)#subscribe

    if result!=MQTT_ERR_SUCCESS:
        logging.critical("Subscribe failed : " + get_MQTT_Error_String(result))
        terminate()
    else:
        logging.info("Subscribe to topic "+topic+" succeeded")

    # we hang around here waiting for callbacks
    # until ctrl-C is pressed
    try:
        while(True):
            time.sleep(1)

    except Exception as e:
        logging.info("Loop terminated - exception :"+e.message)
    finally:
        terminate()


####################################################
#
# start the code if this is the main module
#

if __name__=='__main__':
    main()