import sqlite3


db=sqlite3.connect("mydb")

cursor=db.cursor()

cursor.execute('''SELECT Topic,timestamp,payload,qos FROM MQTT''')

for row in cursor:
    data="{0},{1},{2},{3}\n".format(row[0],row[1],row[2],row[3])
    print data

db.close()