import os
import json
import boto3
from decimal import Decimal
from datetime import datetime, time, timedelta, timezone

def decimal_default(obj):
    if isinstance(obj, Decimal):
        if obj % 1 == 0:
            return int(obj)
        else:
            return float(obj)
    raise TypeError

def lambda_handler(event, context):
    try:
        table_name = os.environ['TABLE_NAME']
        dynamodb = boto3.resource('dynamodb')
        table = dynamodb.Table(table_name)

        filter_param = event.get("queryStringParameters", {}).get("filter", None)

        if filter_param == "today":
            NZ_TZ = timezone(timedelta(hours=12))

            now_nz = datetime.now(NZ_TZ)
            start_of_day = datetime.combine(now_nz.date(), time.min, tzinfo=NZ_TZ)
            end_of_day = datetime.combine(now_nz.date(), time.max, tzinfo=NZ_TZ)

            start_ts = int(start_of_day.astimezone(timezone.utc).timestamp())
            end_ts = int(end_of_day.astimezone(timezone.utc).timestamp())

            response = table.scan()
            items = response.get('Items', [])

            filtered_items = [
                item for item in items
                if start_ts <= int(item.get("timestamp", 0)) <= end_ts
            ]

            return {
                'statusCode': 200,
                'body': json.dumps({'items': filtered_items}, default=decimal_default)
            }

        else:
            response = table.scan()
            items = response.get('Items', [])

            return {
                'statusCode': 200,
                'body': json.dumps({'items': items}, default=decimal_default)
            }

    except Exception as e:
        return {
            'statusCode': 500,
            'body': json.dumps({'error': str(e)})
        }