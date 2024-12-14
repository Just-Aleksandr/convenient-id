import "./App.css";
import axios from "axios";
import { useEffect, useState, useRef } from "react";

function App() {
  const [listOfScans, setListOfScans] = useState([]);
  const [newUser, setNewUser] = useState({
    firstName: "",
    lastName: "",
    rfid: "",
    accessLevel: 1,
  });
  const [showForm, setShowForm] = useState(false);
  const tableWrapperRef = useRef(null); // Reference to the table wrapper for scroll manipulation

  // Fetch the list of scans on component mount
  useEffect(() => {
    axios
      .get("http://localhost:3141/scans")
      .then((response) => {
        setListOfScans(response.data); // Setting the fetched scans with user data
      })
      .catch((error) => {
        console.error(error);
      });
  }, []);

  // Scroll to the bottom instantly when new data is added or on page load
  useEffect(() => {
    scrollToBottom(); // Ensures the table scrolls to the bottom when content is updated
  }, [listOfScans]);

  // Scroll to the bottom function
  const scrollToBottom = () => {
    if (tableWrapperRef.current) {
      tableWrapperRef.current.scrollTop = tableWrapperRef.current.scrollHeight; // Instant scroll to the bottom
    }
  };

  // Handle form input changes
  const handleInputChange = (e) => {
    const { name, value } = e.target;
    setNewUser((prevUser) => ({
      ...prevUser,
      [name]: value,
    }));
  };

  // Handle form submission to add new user
  const handleSubmit = (e) => {
    e.preventDefault();

    // Send the new user data to the backend API
    axios
      .post("http://localhost:3141/users", newUser)
      .then((response) => {
        console.log("User added:", response.data);
        // Optionally, you can trigger a page reload to fetch the new data
        setShowForm(false); // Hide form after submission
        setNewUser({
          firstName: "",
          lastName: "",
          rfid: "",
          accessLevel: 1,
        });
        // Refresh the page to reflect the newly added user
        window.location.reload(); // This will reload the entire page
      })
      .catch((error) => {
        console.error("Error adding user:", error);
        alert("Failed to add user");
      });
  };

  return (
    <div className="App">
      {/* Display the table of scans */}
      <div className="table-wrapper" ref={tableWrapperRef}>
        <table>
          <thead>
            <tr>
              <th>Created At</th>
              <th>RFID</th>
              <th>First Name</th>
              <th>Last Name</th>
              <th>Access Level</th>
              <th>Authorization Status</th>
            </tr>
          </thead>
          <tbody>
            {listOfScans.map((scan, key) => (
              <tr key={key}>
                <td>{new Date(scan.createdAt).toLocaleString()}</td>
                <td>{scan.rfid}</td>
                <td>{scan.user ? scan.user.firstName : "N/A"}</td>
                <td>{scan.user ? scan.user.lastName : "N/A"}</td>
                <td>{scan.user ? scan.user.accessLevel : "N/A"}</td>
                <td
                  style={{
                    color: scan.authorized ? "green" : "red",
                  }}
                >
                  {scan.authorized ? "Authorized" : "Unauthorized"}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>

      {/* Show the form to add a new user */}
      {showForm && (
        <form onSubmit={handleSubmit} className="user-form">
          <div>
            <label htmlFor="firstName">First Name</label>
            <input
              type="text"
              id="firstName"
              name="firstName"
              value={newUser.firstName}
              onChange={handleInputChange}
              required
            />
          </div>
          <div>
            <label htmlFor="lastName">Last Name</label>
            <input
              type="text"
              id="lastName"
              name="lastName"
              value={newUser.lastName}
              onChange={handleInputChange}
              required
            />
          </div>
          <div>
            <label htmlFor="rfid">RFID</label>
            <input
              type="text"
              id="rfid"
              name="rfid"
              value={newUser.rfid}
              onChange={handleInputChange}
              required
            />
          </div>
          <div>
            <label htmlFor="accessLevel">Access Level</label>
            <input
              type="number"
              id="accessLevel"
              name="accessLevel"
              value={newUser.accessLevel}
              onChange={handleInputChange}
              min="1"
              max="10"
              required
            />
          </div>
          <button type="submit">Submit</button>
        </form>
      )}

      {/* Button for adding new user, toggles between Add New User and Cancel */}
      <button
        onClick={() => {
          if (showForm) {
            setShowForm(false); // Hide the form when Cancel is pressed
          } else {
            setShowForm(true); // Show the form when Add New User is pressed
          }
        }}
        className="add-user-btn"
      >
        {showForm ? "Cancel" : "Add New User"}
      </button>
    </div>
  );
}

export default App;
