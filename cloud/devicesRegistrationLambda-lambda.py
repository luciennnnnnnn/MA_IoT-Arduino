# Importing required packages
import json
import boto3

# Function definition
def lambda_handler(event, context):
    # Initialize the DynamoDB resource
    dynamodb = boto3.resource('dynamodb')
    
    # Table name
    table = dynamodb.Table('devicesIDTable')  # Update with your table name

    try:
        # Parse the incoming event body
        body = event['body']
        user_id = body.get('userId')  # Retrieve userId
        device_id = body.get('deviceId')  # Retrieve deviceID

        # Validate inputs
        if not user_id or not device_id:
            return {
                "statusCode": 401,
                "body": json.dumps({"message": "User ID and device ID are required"})
            }

        # Check if the pair (userId, deviceId) already exists in the table
        response = table.query(
            KeyConditionExpression='UserID = :user_id and DeviceID = :device_id',
            ExpressionAttributeValues={
                ':user_id': user_id,
                ':device_id': device_id
            }
        )

        # If the query returns any items, the pair already exists
        if response['Items']:
            return {
                "statusCode": 400,
                "body": json.dumps({"message": "This user ID and device ID combination already exists"})
            }
        
        # Insert values into the table
        response = table.put_item(
            Item={
                'UserID': user_id,  # Partition key
                'DeviceID': device_id,  # Sort key
            }
        )

        # Return a success response
        return {
            "statusCode": 200,
            "body": json.dumps({"message": "Data written successfully", "response": response})
        }

    except Exception as e:
        # Handle and log errors
        print(f"Error: {str(e)}")
        return {
            "statusCode": 500,
            "body": json.dumps({"message": "An error occurred", "error": str(e)})
        }
