import "./App.css";
import axios from "axios";
import { useEffect, useState, useRef } from "react";

function App() {
  const [listOfScans, setListOfScans] = useState([]);
  const tableRef = useRef(null); // Reference to the scrollable table container

  useEffect(() => {
    axios
      .get("http://localhost:3141/scans")
      .then((response) => {
        setListOfScans(response.data);
      })
      .catch((error) => {
        console.error(error);
      });
  }, []);

  // Scroll to the bottom of the table after data is loaded
  useEffect(() => {
    if (tableRef.current) {
      tableRef.current.scrollTop = tableRef.current.scrollHeight;
    }
  }, [listOfScans]);

  // Function to get the class for authorization status cell
  const getStatusClass = (authorized) => {
    return authorized ? "authorized-status" : "unauthorized-status";
  };

  return (
    <div className="App">
      <div ref={tableRef} className="table-container">
        <table className="table">
          <thead>
            <tr>
              <th>Timestamp</th>
              <th>RFID</th>
              <th>First Name</th>
              <th>Last Name</th>
              <th>Access Level</th>
              <th>Status</th>
            </tr>
          </thead>
          <tbody>
            {listOfScans.map((value, key) => (
              <tr key={key}>
                <td>{new Date(value.createdAt).toLocaleString()}</td>
                <td>{value.rfid}</td>
                {value.user ? (
                  <>
                    <td>{value.user.firstName}</td>
                    <td>{value.user.lastName}</td>
                    <td>{value.user.accessLevel}</td>
                  </>
                ) : (
                  <>
                    <td>N/A</td>
                    <td>N/A</td>
                    <td>N/A</td>
                  </>
                )}
                <td className={getStatusClass(value.authorized)}>
                  {value.authorized ? "Authorized" : "Unauthorized"}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

export default App;
