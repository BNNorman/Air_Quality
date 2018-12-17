#
#
#
# sucks everything from the MQTT table and dumps to CSV

import sqlite3

storeDBfile="mydb"
CSV_file="mqtt.csv"

db=sqlite3.connect(storeDBfile)

csv=open(CSV_file,"w+")

cursor=db.cursor()

cursor.execute('''SELECT Topic,timestamp,payload,qos FROM MQTT''')

csv.write("Topic,Timestamp,Payload,Qos\n")

for row in cursor:
    data="{0},[1},{2},{3}\n".format(row[0],row[1],row[2],row[3])
    csv.write(data)

csv.close()
db.close()
