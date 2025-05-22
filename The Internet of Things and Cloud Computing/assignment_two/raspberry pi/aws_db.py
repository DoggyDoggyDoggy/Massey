import boto3
from datetime import datetime, timezone

dynamodb = boto3.resource('dynamodb', region_name = 'ap-southeast-2')
table = dynamodb.Table('SoilMoisture')


def put_to_dynamo(device_id, soilmoisture):
    timestamp = int(datetime.now(timezone.utc).timestamp())
    try:
        table.put_item(Item={
            'deviceId': device_id,
            'timestamp': timestamp,
            'soilmoisture': soilmoisture
        })
        print(f"[+] Uploaded: {device_id} - {soilmoisture} at {timestamp}")
    except Exception as e:
        print(f"[!] Error sending to DynamoDB: {e}")
