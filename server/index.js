const express = require("express");
const app = express();

app.use(express.json());

const db = require("./models");

// Routers
const usersRouter = require("./routes/users");
const { json } = require("sequelize");
app.use("/users", usersRouter);

db.sequelize.sync().then(() => {
  app.listen(3141, () => {
    console.log("Server running on port 3141");
  });
});
