/* Data validation schemas */

db.createCollection("users", {
    validator: {
        $jsonSchema: {
            bsonType: "object",
            additionalProperties: false,
            required: [ "_id", "login", "first_name", "last_name" ],
            properties: {
                _id: { bsonType: "binData" },
                login: { bsonType: "string" },
                first_name: { bsonType: "string" },
                last_name: { bsonType: "string" }
            }
        }
    }
})

db.createCollection("credentials", {
    validator: {
        $jsonSchema: {
            bsonType: "object",
            additionalProperties: false,
            required: [ "_id", "verify_secret", "user_id" ],
            properties: {
                _id: { bsonType: "string" },
                verify_secret: { bsonType: "string" },
                user_id: { bsonType: "binData" },
            },
        },
    },
});

db.createCollection("properties", {
    validator: {
        $jsonSchema: {
            bsonType: "object",
            additionalProperties: false,
            required: [ "_id", "price", "status", "owner_id", "address" ],
            properties: {
                _id: { bsonType: "binData" },
                price: { bsonType: "int", minimum: 0 },
                status: { bsonType: "string", enum: [ "active", "sold" ] },
                owner_id: { bsonType: "binData" },
                address: {
                    bsonType: "object",
                    additionalProperties: false,
                    required: [ "country", "city", "street", "building" ],
                    properties: {
                        country: { bsonType: "string" },
                        city: { bsonType: "string" },
                        street: { bsonType: "string" },
                        building: { bsonType: "int", minimum: 1 },
                        apartment: { bsonType: "int", minimum: 1 }
                    }
                }
            }
        },
    },
});

db.createCollection("viewings", {
    validator: {
        $jsonSchema: {
            bsonType: "object",
            additionalProperties: false,
            required: [ "_id", "user_id", "property_id", "viewing_date" ],
            properties: {
                _id: { bsonType: "binData" },
                user_id: { bsonType: "binData" },
                property_id: { bsonType: "binData" },
                viewing_date: { bsonType: "date" },
            }
        },
    },
});

/* Indexes */

db.viewings.createIndex({ property_id: 1, viewing_date: 1 }, { unique: 1, name: "index_date_unique_per_property" });
db.users.createIndex({ login: 1 }, { unique: 1, name: "index_user_login_unique" });
