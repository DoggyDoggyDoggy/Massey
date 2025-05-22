import json
import boto3

iot = boto3.client('iot-data')

def lambda_handler(event, context):
    try:
        body = json.loads(event["body"])
        device_id = body["device_id"]

        topic = f"soil/{device_id}/read"
        payload = json.dumps({"cmd": "read"})

        iot.publish(topic=topic, qos=1, payload=payload)

        return {
            "statusCode": 200,
            "body": json.dumps({"message": "Command sent"})
        }

    except Exception as e:
        return {
            "statusCode": 500,
            "body": json.dumps({"error": str(e)})
        }