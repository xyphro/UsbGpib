import pyvisa
import time
import paho.mqtt.client as mqtt

# MQTT broker settings
broker = '192.168.0.110'  # Replace with your broker address
port = 1883                  # Replace with your broker's port if different
topic = "test/topic"         # Replace with your topic

# Initialize MQTT client
client = mqtt.Client()

# Connect to the broker
client.connect(broker, port)
client.loop_start()  # Start the loop in a separate thread for automatic reconnection

# Initialize VISA resource manager
rm = pyvisa.ResourceManager()
# Update this to your instrument's VISA address
visa_address = 'USB0::0x03EB::0x2065::HEWLETT-PACKARD_34401A_0_10-5-2::INSTR'  # Replace with your instrument's address
instrument = rm.open_resource(visa_address)

# Clear any previous errors or data and configure the instrument
instrument.write('*CLS')
instrument.write('CONF:VOLT:DC')  # Configure to measure DC voltage
time.sleep(0.2)  # Delay after initial configuration

try:
    while True:
        # Read voltage from the instrument
        response = instrument.query('READ?')
        
        try:
            # Parse response as float and publish
            voltage = float(response.strip())
            result = client.publish(topic, voltage)
            
            # Check publish result
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                print(f"Sent `{voltage}` to topic `{topic}`")
            else:
                print(f"Failed to send message to topic {topic}, MQTT result code: {result.rc}")
        
        except ValueError as e:
            print(f"Error parsing response: {response} -> {e}")

        # Short delay between readings
        time.sleep(0.1)  # Adjust as necessary

finally:
    # Cleanup
    instrument.close()
    client.loop_stop()
    client.disconnect()
