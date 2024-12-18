import boto3
import json
from boto3.dynamodb.conditions import Key

# Initialize the DynamoDB resource
dynamodb = boto3.resource('dynamodb')

# Table name
table = dynamodb.Table('devicesIDTable')

def lambda_handler(event, context):
    try:
        # Extract query string parameter
        params = event.get('queryStringParameters', {})
        user_id = params.get('userID')

        if not user_id:
            return {
                'statusCode': 400,  # Returning 400 for a bad request
                'body': json.dumps({"message": "userID query parameter is required"})
            }

        # Query DynamoDB for items with the given UserID
        response = table.query(
            KeyConditionExpression=Key('UserID').eq(user_id)
        )

        # Extract the device IDs into a simple list
        device_ids = [item['DeviceID'] for item in response.get('Items', [])]

        # Return the device IDs directly without 'body' or other extra structure
        return device_ids  # Returning the list of device IDs directly
    
    except Exception as e:
        # Handle and log errors
        print(f"Error: {str(e)}")
        return {
            "statusCode": 500,
            "body": json.dumps({"message": "An error occurred", "error": str(e)})
        }
