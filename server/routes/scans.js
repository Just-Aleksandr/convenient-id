const express = require("express");
const router = express.Router();
const { users, scans } = require("../models");
const { check, validationResult } = require("express-validator");

router.get("/", async (req, res) => {
  const listOfScans = await getScansWithUserDetails();
  res.json(listOfScans);
});

/*
router.post("/", async (req, res) => {
  const user = req.body;
  try {
    await users.create(user);
    res.json(user);
  } catch (error) {
    console.error(error);
    res.status(400).json(error);
  }
});*/

async function getScansWithUserDetails() {
  const results = await scans.findAll({
    include: [
      {
        model: users,
        attributes: ["firstName", "lastName", "accessLevel"], // Specify the fields to include from `users`
      },
    ],
  });

  return results;
}

module.exports = router;
