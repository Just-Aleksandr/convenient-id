const express = require("express");
const router = express.Router();
const { users } = require("../models");
const { check, validationResult } = require("express-validator");

router.get("/", [check("rfid").exists()], async (req, res) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).send();
    // return res.status(400).json({ errors: errors.array() });
  }
  const result = await users.findOne({
    where: {
      rfid: req.query.rfid,
    },
  });

  res.json(!!result);
});

router.post("/", async (req, res) => {
  const user = req.body;
  try {
    await users.create(user);
    res.json(user);
  } catch (error) {
    console.error(error);
    res.status(400).json(error);
  }
});

module.exports = router;
