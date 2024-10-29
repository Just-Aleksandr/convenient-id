module.exports = (sequelize, DataTypes) => {
  const scans = sequelize.define("users", {
    rfid: {
      type: DataTypes.STRING,
      allowNull: false,
      primaryKey: true,
    },
    firstName: {
      type: DataTypes.STRING,
      allowNull: false,
    },
    lastName: {
      type: DataTypes.STRING,
      allowNull: false,
    },
    accessLevel: {
      type: DataTypes.INTEGER,
      allowNull: false,
    },
  });

  return scans;
};
