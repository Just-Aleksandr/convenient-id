module.exports = (sequelize, DataTypes) => {
  const users = sequelize.define("users", {
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

  users.associate = (models) => {
    users.hasMany(models.scans, { foreignKey: "rfid", sourceKey: "rfid" });
  };

  return users;
};
