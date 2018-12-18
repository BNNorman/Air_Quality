#############################################
#
# MQTT subscriber with mariaDB (mySql) storage
#
# Author: Brian Norman Dec 2018
#
# License: free to use and abuse
#
#
#
#MariaDB [MQTT]> describe Data;
#+-----------+---------------+------+-----+---------+-------+
#| Field     | Type          | Null | Key | Default | Extra |
#+-----------+---------------+------+-----+---------+-------+
#| Topic     | text          | YES  |     | NULL    |       |
#| TimeStamp | decimal(10,0) | YES  |     | NULL    |       |
#| Payload   | text          | YES  |     | NULL    |       |
#| Qos       | int(11)       | YES  |     | NULL    |       |
#+-----------+---------------+------+-----+---------+-------+
#4 rows in set (0.00 sec)

#
import time
import mysql.connector
import paho.mqtt.client as paho
from MQTT_Errors import *
import logging

#############################################
#
# config
#

broker="192.168.1.44"   # PINAT
dbName="MQTT"
dbHost=broker
dbUser="brian"
dbPassword="xxxxxxxxx"
logFile="mqtt.log"
topic="#"   # subscribe to eevrything

verbose=False

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
    global topic

    if verbose:
        print "on_connect callback"

    if rc==0:
        logging.info("on_connect: Connection successful")
        (result,mid)=client.subscribe(topic)
        if result != MQTT_ERR_SUCCESS:
            logging.info("Subscribe to topic "+topic+" failed : " + get_MQTT_Error_String(result))
        else:
            logging.info("Subscribe to topic " + topic + " succeeded")
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

    if verbose:
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
        db = mysql.connector.connect(
            host=dbHost,
            user=dbUser,
            passwd=dbPassword,
            database=dbName
        )

    except Exception as e:
        logging.info("connect to " + dbName + "@" + dbHost + "failed.")
        logging.info(e)
        terminate()

    try:
        cursor=db.cursor()
        cursor.execute('''INSERT INTO Data(Topic,TimeStamp,Payload,Qos)
               VALUES(%s,%s,%s,%s)''', (message.topic,timestamp, message.payload.decode("utf-8"), message.qos))
        db.commit()
        db.close()
        logging.info("Added message to database.")

    except Exception as e:
        logging.info("SQL INSERT failed reason:-")
        logging.info(e)
        terminate()



def on_subscribe(client, userdata, mid, qos):
    # just log the fact that the subscription took place
    if verbose:
        print "on_subscribe callback"

    logging.info("on_subscribe callback")


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

    global client, storeDBfile, broker
    ##############################################
    #
    # connect to the database - will create a file if it doesn't exist
    #
    try:
        db = mysql.connector.connect(
            host=dbHost,
            user=dbUser,
            passwd=dbPassword,
            database=dbName
        )

        try:
            cursor = db.cursor()
            # check if the table exists
            cursor.execute("SELECT table_name FROM information_schema.tables WHERE table_schema = 'MQTT' AND "
                           "table_name = 'Data'")
            if cursor.rowcount==0:
                logging.info("MQTT Data table not found.")
                terminate()

        except Exception as e:
            logging.info("Unable to determine if Data table exists")
            logging.info(e)
            db.close()
            terminate()

        finally:
            # the database has to be re-opened in the on_message callback
            # which is in a seperate thread
            db.close()
    except Exception as e:
        logging.info("Unable to connect to database "+dbName+"@"+dbHost)
        logging.info(e)
        terminate()

    ################################################
    #
    # create the paho MQTT client, set the callbacks
    # and start the client loop
    #

    try:
        client=paho.Client() # paho allocates a random unique clientID

        client.connect(broker)  # connect

        # now setup MQTT callbacks
        client.on_message = on_message
        client.on_connect = on_connect
        client.on_subscribe = on_subscribe
        if verbose:
            client.on_log = on_log
        client.loop_start()  # start loop to process received messages

    except Exception as e:
        logging.critical("unable to create paho client or connect. Err="+e.message)
        terminate()


    #############################################
    #
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
