db.users.insertMany([
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b0", "login": "user_01", "first_name": "Alexey", "last_name": "Grishin"},
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b1", "login": "user_02", "first_name": "Maria", "last_name": "Petrova"},
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b2", "login": "user_03", "first_name": "Dmitry", "last_name": "Sokolov"},
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b3", "login": "user_04", "first_name": "Elena", "last_name": "Kuznetsova"},
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b4", "login": "user_05", "first_name": "Ivan", "last_name": "Smirnov"},
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b5", "login": "user_06", "first_name": "Anna", "last_name": "Volkova"},
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b6", "login": "user_07", "first_name": "Sergey", "last_name": "Popov"},
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b7", "login": "user_08", "first_name": "Olga", "last_name": "Mikhailova"},
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b8", "login": "user_09", "first_name": "Nikita", "last_name": "Fedorov"},
    {"_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b9", "login": "user_10", "first_name": "Victoria", "last_name": "Orlova"}
]);

db.credentials.insertMany([
    {"_id": "user_01", "verify_secret": "user_01", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b0"},
    {"_id": "user_02", "verify_secret": "user_02", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b1"},
    {"_id": "user_03", "verify_secret": "user_03", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b2"},
    {"_id": "user_04", "verify_secret": "user_04", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b3"},
    {"_id": "user_05", "verify_secret": "user_05", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b4"},
    {"_id": "user_06", "verify_secret": "user_06", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b5"},
    {"_id": "user_07", "verify_secret": "user_07", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b6"},
    {"_id": "user_08", "verify_secret": "user_08", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b7"},
    {"_id": "user_09", "verify_secret": "user_09", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b8"},
    {"_id": "user_10", "verify_secret": "user_10", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b9"}
]);

db.properties.insertMany([
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745400",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b0",
        "status": "active",
        "price": 100,
        "address": {"country": "Russia", "city": "Moscow", "street": "Tverskaya", "building": 1},
    },
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745401",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b1",
        "status": "active",
        "price": 200,
        "address": {"country": "Russia", "city": "Moscow", "street": "Arbat", "building": 15},
    },
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745402",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b2",
        "status": "active",
        "price": 300,
        "address": {"country": "Russia", "city": "Saint Petersburg", "street": "Nevsky Prospekt", "building": 28},
    },
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745403",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b3",
        "status": "active",
        "price": 400,
        "address": {"country": "Russia", "city": "Kazan", "street": "Baumana", "building": 42},
    },
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745404",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b4",
        "status": "active",
        "price": 500,
        "address": {"country": "Russia", "city": "Novosibirsk", "street": "Krasny Prospekt", "building": 7},
    },
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745405",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b5",
        "status": "sold",
        "price": 600,
        "address": {"country": "Russia", "city": "Yekaterinburg", "street": "Lenina", "building": 54, "apartment": 1},
    },
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745406",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b6",
        "status": "sold",
        "price": 700,
        "address": {"country": "Russia", "city": "Moscow", "street": "Leninsky Prospekt", "building": 33, "apartment": 2},
    },
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745407",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b7",
        "status": "sold",
        "price": 800,
        "address": {"country": "Russia", "city": "Sochi", "street": "Kurortny Prospekt", "building": 12, "apartment": 3},
    },
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745408",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b8",
        "status": "sold",
        "price": 900,
        "address": {"country": "Russia", "city": "Vladivostok", "street": "Svetlanskaya", "building": 89, "apartment": 4},
    },
    {
        "_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745409",
        "owner_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b9",
        "status": "sold",
        "price": 1000,
        "address": {"country": "Russia", "city": "Moscow", "street": "Myasnitskaya", "building": 20, "apartment": 5},
    }
]);

db.viewings.insertMany([
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0950", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b1", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745400", "viewing_date": "2026-04-01"},
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0951", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b2", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745401", "viewing_date": "2026-04-02"},
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0952", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b3", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745402", "viewing_date": "2026-04-03"},
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0953", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b4", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745403", "viewing_date": "2026-04-01"},
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0954", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b5", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745404", "viewing_date": "2026-04-04"},
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0955", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b6", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745405", "viewing_date": "2026-04-05"},
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0956", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b7", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745406", "viewing_date": "2026-04-02"},
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0957", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b8", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745407", "viewing_date": "2026-04-06"},
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0958", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b9", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745408", "viewing_date": "2026-04-07"},
    {"_id": "caea098d-88ac-4866-8a2b-41167e1d0959", "user_id": "d745f221-ddcb-4e4f-b3c7-4f931f5576b0", "property_id": "a907b5ad-8e1e-49a2-b252-8ab4ba745409", "viewing_date": "2026-04-08"}
]);
