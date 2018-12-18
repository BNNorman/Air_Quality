import mysql.connector


broker="192.168.1.44"   # PINAT
dbName="MQTT"
dbHost=broker
dbUser="brian"
dbPassword="xxxxxxx"

db = mysql.connector.connect(
    host=dbHost,
    user=dbUser,
    passwd=dbPassword,
    database=dbName
)

cursor=db.cursor()

cursor.execute('''SELECT * FROM Data''')

for row in cursor:
    data="{0},{1},{2},{3}\n".format(row[0],row[1],row[2],row[3])
    print data

db.close()
