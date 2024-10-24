from flask import Flask, request, jsonify
import boto3
import json
from flask_cors import CORS
app = Flask(__name__)
CORS(app)
# Initialize AWS IoT Data client
iot_client = boto3.client('iot-data', region_name='ap-southeast-2')


@app.route('/fan-control', methods=['POST'])
def control_fan():
    # Get the fan status from the incoming request
    data = request.json
    fan_status = data.get('fan_status')

    # Validate fan status
    if fan_status not in ['ON', 'OFF']:
        return jsonify({'message': 'Invalid fan status'}), 400

    # Publish the fan status to AWS IoT Core
    iot_client.publish(
        topic='device/control',
        qos=1,
        payload=json.dumps({'fan_status': fan_status})
    )

    # Return success response
    return jsonify({'message': f'Fan {fan_status} command sent successfully'})


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
