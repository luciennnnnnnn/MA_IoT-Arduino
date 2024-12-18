import { useState, useEffect } from "react";
import './App.css';

import { Amplify } from 'aws-amplify';
import { withAuthenticator } from '@aws-amplify/ui-react';

import awsconfig from './aws-exports';
Amplify.configure(awsconfig);

function App({ signOut, user }) {
  const [deviceId, setDeviceId] = useState("");
  const [message, setMessage] = useState("");
  const [registeredDevices, setRegisteredDevices] = useState([]);

  // Function to fetch registered devices
  const fetchRegisteredDevices = async () => {
    try {
      const response = await fetch(
        `https://g5qs0s738b.execute-api.eu-west-1.amazonaws.com/prod/getDevices?userID=${encodeURIComponent(user?.signInDetails?.loginId)}`
      );

      if (response.ok) {
        const devices = await response.json(); // Directly parse response as JSON array
        setRegisteredDevices(devices);
      } else {
        console.error("Failed to fetch devices.");
      }
    } catch (error) {
      console.error("Error fetching devices:", error);
    }
  };

  // Function to register a new device
  const registerDevice = async () => {
    try {
      const requestBody = {
        userId: user?.signInDetails?.loginId,
        deviceId: deviceId,
      };
      const response = await fetch(
        "https://g5qs0s738b.execute-api.eu-west-1.amazonaws.com/prod/deviceRegistrationLambda",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify(requestBody),
        }
      );

      const responseBody = await response.json();

      if (response.ok && responseBody.statusCode === 200) {
        setMessage("Device registered successfully!");
        fetchRegisteredDevices(); // Refresh the list
      } else if (responseBody.statusCode === 400) {
        setMessage("Device ID already registered.");
      } else if (responseBody.statusCode === 401) {
        setMessage("Device ID is required.");
      } else {
        setMessage("Failed to register device.");
      }
    } catch (error) {
      console.error(error);
      setMessage("Error registering device.");
    }
  };

  // Fetch devices on component mount
  useEffect(() => {
    fetchRegisteredDevices();
  }, [user]);

  return (
    <div>
      <h1>New Device Registration</h1>
      <input
        type="text"
        placeholder="Enter device ID"
        value={deviceId}
        onChange={(e) => setDeviceId(e.target.value)}
      />
      <button onClick={registerDevice}>Register Device</button>
      <p>{message}</p>

      <h2>Registered Devices</h2>
      {registeredDevices.length > 0 ? (
        <ul>
          {registeredDevices.map((device, index) => (
            <li key={index}>{device}</li>
          ))}
        </ul>
      ) : (
        <p>No devices registered yet.</p>
      )}
    </div>
  );
}

export default withAuthenticator(App);
