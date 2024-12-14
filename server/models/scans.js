module.exports = (sequelize, DataTypes) => {
  const scans = sequelize.define(
    "scans",
    {
      id: {
        type: DataTypes.INTEGER,
        autoIncrement: true,
        primaryKey: true,
      },
      rfid: {
        type: DataTypes.STRING,
        allowNull: false,
      },
      authorized: {
        type: DataTypes.BOOLEAN,
        allowNull: false,
      },
    },
    {
      timestamps: true,
      createdAt: true,
      updatedAt: false,
    }
  );

  scans.associate = (models) => {
    scans.belongsTo(models.users, { foreignKey: "rfid", targetKey: "rfid" });
  };

  return scans;
};
