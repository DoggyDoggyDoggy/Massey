import boto3
from datetime import datetime

dynamodb = boto3.resource('dynamodb', region_name='ap-southeast-2')
table = dynamodb.Table('SoilMoisture')

def put_to_dynamo(device_id):
    timestamp = int(datetime.utcnow().timestamp())
    table.put_item(Item={
        'deviceId': device_id,
        'timestamp': timestamp,
        'soilmoisture': 15
    })

put_to_dynamo("py_test")
print("done")
