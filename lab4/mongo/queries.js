// User storage

const createUser = (login, firstName, lastName) => {
    return db.users.insertOne({
        _id: new UUID(),
        login: login,
        first_name: firstName,
        last_name: lastName,
    });
};

const getUser = (userId) => {
    return db.users.findOne({_id: new UUID(userId)});
}

const findUsers = (login = null, first_name = null, last_name = null) => {
    const filter = {};
    if (login !== null) filter["login"] = { "$regex": login };
    if (first_name !== null) filter["first_name"] = { "$regex": first_name };
    if (last_name !== null) filter["last_name"] = { "$regex": last_name };

    return db.users.find(filter);
};

// Credentials storage

const registerUser = (login, verify_secret, user_id) => {
    return db.credentials.insertOne({
        _id: login,
        verify_secret: verify_secret,
        user_id: new UUID(user_id),
    });
};

const verifyCredentials = (key, verify_secret) => {
    return db.credentials.findOne(
        { _id: key, verify_secret: verify_secret },
        { user_id: 1, _id: 0 }
    );
};

// Property storage

const createProperty = (propertyStatus, price, street) => {
    return db.properties.insertOne({
        _id: new UUID(),
        status: propertyStatus,
        price: price,
        street: street,
    });
};

const updateProperty = (id, price = null, propertyStatus = null) => {
    const updatedFields = {};
    if (price !== null) updatedFields.price = price;
    if (propertyStatus !== null) updatedFields.status = propertyStatus;

    return db.properties.updateOne(
        {_id: id},
        { $set: updatedFields }
    );
};

const getProperty = (id) => {
    return db.properties.findOne({_id: new UUID(id)});
};

const findProperties = (city = null, minPrice = null, maxPrice = null, ownerId = null) => {
    const filter = {};
    if (city !== null) filter["address.city"] = { $regex: city };
    if (minPrice !== null || maxPrice !== null) {
        filter.price = {};
        if (minPrice !== null) filter.price["$gte"] = minPrice;
        if (maxPrice !== null) filter.price["$lte"] = maxPrice;
    }
    if (ownerId !== null) filter.owner_id = new UUID(ownerId);

    return db.properties.find(filter, { _id: 1 });
};

// Viewings storage

const createViewing = (userId, propertyId, viewingDate) => {
    return db.viewings.insertOne({
        _id: new UUID(),
        user_id: new UUID(userId),
        property_id: new UUID(propertyId),
        viewing_date: new Date(viewingDate),
    });
};

const deleteViewing = (viewingId) => {
    return db.viewings.deleteOne({ _id: new UUID(viewingId) });
};

const getViewing = (viewingId) => {
    return db.viewings.findOne({ _id: new UUID(viewingId) });
};

const findViewings = (userId = null, propertyId = null) => {
    const filter = {};
    if (userId !== null) filter.user_id = new UUID(userId);
    if (propertyId !== null) filter.property_id = new UUID(propertyId);

    return db.viewings.find(filter, { _id: 1 });
};
