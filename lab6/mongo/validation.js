/* Users storage */

// Не указаны обязательные поля 'login', 'first_name', 'last_name'
db.users.insertOne({ _id: new UUID() })

// Поле 'login' должно иметь значение типа 'string'
db.users.insertOne({ _id: new UUID(), login: 23, first_name: "Alexey", last_name: "Grishin" })

/* Credentials storage */

// Поле '_id' должно быть типа 'string'
db.credentials.insertOne({ _id: 23, verify_secret: "foo", user_id: new UUID() })

// Поле 'verify_secret' должно содержать строку
db.credentials.insertOne({ _id: "key", verify_secret: new UUID(), user_id: new UUID() })

// Не указано обязательное поле 'user_id'
db.credentials.insertOne({ _id: "key", verify_secret: "secret" })

// Лишнее поле 'extra'
db.credentials.insertOne({ _id: "key", verify_secret: "secret", user_id: new UUID(), extra: 23 })

/* Properties storage */

// Цена объекта недвижимости не может быть отрицательной
db.properties.insertOne({
    _id: new UUID(),
    price: -200,
    status: "active",
    owner_id: new UUID(),
    address: { country: "Russia", city: "Moscow", street: "Sedova", "building": 34 },
});


// Неизвестное состояние объекта недвижимости
db.properties.insertOne({
    _id: new UUID(),
    price: 200,
    status: "unknown",
    owner_id: new UUID(),
    address: { country: "Russia", city: "Moscow", street: "Sedova", "building": 34 },
});

// Невалидное значение адреса объекта недвижимости
db.properties.insertOne({
    _id: new UUID(),
    price: 200,
    status: "unknown",
    owner_id: new UUID(),
    address: { invalid: "value" },
});

/* Viewings storage */

// Поле 'viewing_date' должно содержать строго значение типа 'Date'
db.viewings.insertOne({ _id: new UUID(), user_id: new UUID(), property_id: new UUID(), viewing_date: "2026-04-13" });

// Нельзя записаться на осмотр одного объекта в один и тот же день дважды
const propertyId = db.properties.findOne()._id;
db.viewings.insertOne({ _id: new UUID(), user_id: new UUID(), property_id: propertyId, viewing_date: new Date("2026-04-14") });
db.viewings.insertOne({ _id: new UUID(), user_id: new UUID(), property_id: propertyId, viewing_date: new Date("2026-04-14") });
