INSERT INTO users (id, login, first_name, last_name) VALUES
    (md5('user_01')::uuid, 'user_01', 'Alexey', 'Grishin'),
    (md5('user_02')::uuid, 'user_02', 'Maria', 'Petrova'),
    (md5('user_03')::uuid, 'user_03', 'Dmitry', 'Sokolov'),
    (md5('user_04')::uuid, 'user_04', 'Elena', 'Kuznetsova'),
    (md5('user_05')::uuid, 'user_05', 'Ivan', 'Smirnov'),
    (md5('user_06')::uuid, 'user_06', 'Anna', 'Volkova'),
    (md5('user_07')::uuid, 'user_07', 'Sergey', 'Popov'),
    (md5('user_08')::uuid, 'user_08', 'Olga', 'Mikhailova'),
    (md5('user_09')::uuid, 'user_09', 'Nikita', 'Fedorov'),
    (md5('user_10')::uuid, 'user_10', 'Victoria', 'Orlova');

INSERT INTO addresses (id, country, city, street, building, apartment) VALUES
    (md5('addr_01')::uuid, 'Russia', 'Moscow', 'Tverskaya', 1, NULL),
    (md5('addr_02')::uuid, 'Russia', 'Moscow', 'Arbat', 15, NULL),
    (md5('addr_03')::uuid, 'Russia', 'Saint Petersburg', 'Nevsky Prospekt', 28, NULL),
    (md5('addr_04')::uuid, 'Russia', 'Kazan', 'Baumana', 42, NULL),
    (md5('addr_05')::uuid, 'Russia', 'Novosibirsk', 'Krasny Prospekt', 7, NULL),
    (md5('addr_06')::uuid, 'Russia', 'Yekaterinburg', 'Lenina', 54, 1),
    (md5('addr_07')::uuid, 'Russia', 'Moscow', 'Leninsky Prospekt', 33, 2),
    (md5('addr_08')::uuid, 'Russia', 'Sochi', 'Kurortny Prospekt', 12, 3),
    (md5('addr_09')::uuid, 'Russia', 'Vladivostok', 'Svetlanskaya', 89, 4),
    (md5('addr_10')::uuid, 'Russia', 'Moscow', 'Myasnitskaya', 20, 5);

INSERT INTO credentials (key, verify_secret, user_id) VALUES
    ('user_01', md5('user_01'), md5('user_01')::uuid),
    ('user_02', md5('user_02'), md5('user_02')::uuid),
    ('user_03', md5('user_03'), md5('user_03')::uuid),
    ('user_04', md5('user_04'), md5('user_04')::uuid),
    ('user_05', md5('user_05'), md5('user_05')::uuid),
    ('user_06', md5('user_06'), md5('user_06')::uuid),
    ('user_07', md5('user_07'), md5('user_07')::uuid),
    ('user_08', md5('user_08'), md5('user_08')::uuid),
    ('user_09', md5('user_09'), md5('user_09')::uuid),
    ('user_10', md5('user_10'), md5('user_10')::uuid);

INSERT INTO properties (id, owner_id, address_id, status, price) VALUES
    (md5('property_01')::uuid, md5('user_01')::uuid, md5('addr_01')::uuid, 'active', 100),    
    (md5('property_02')::uuid, md5('user_02')::uuid, md5('addr_02')::uuid, 'active', 200),    
    (md5('property_03')::uuid, md5('user_03')::uuid, md5('addr_03')::uuid, 'active', 300),    
    (md5('property_04')::uuid, md5('user_04')::uuid, md5('addr_04')::uuid, 'active', 400),    
    (md5('property_05')::uuid, md5('user_05')::uuid, md5('addr_05')::uuid, 'active', 500),    
    (md5('property_06')::uuid, md5('user_06')::uuid, md5('addr_06')::uuid, 'sold', 600),    
    (md5('property_07')::uuid, md5('user_07')::uuid, md5('addr_07')::uuid, 'sold', 700),    
    (md5('property_08')::uuid, md5('user_08')::uuid, md5('addr_08')::uuid, 'sold', 800),    
    (md5('property_09')::uuid, md5('user_09')::uuid, md5('addr_09')::uuid, 'sold', 900),    
    (md5('property_10')::uuid, md5('user_10')::uuid, md5('addr_10')::uuid, 'sold', 1000);

INSERT INTO viewings (id, user_id, property_id, viewing_date) VALUES
    (md5('viewing_01')::uuid, md5('user_01')::uuid, md5('property_10')::uuid, '2026-04-01'),
    (md5('viewing_02')::uuid, md5('user_02')::uuid, md5('property_01')::uuid, '2026-04-02'),
    (md5('viewing_03')::uuid, md5('user_03')::uuid, md5('property_02')::uuid, '2026-04-03'),
    (md5('viewing_04')::uuid, md5('user_04')::uuid, md5('property_03')::uuid, '2026-04-01'),
    (md5('viewing_05')::uuid, md5('user_05')::uuid, md5('property_04')::uuid, '2026-04-04'),
    (md5('viewing_06')::uuid, md5('user_06')::uuid, md5('property_05')::uuid, '2026-04-05'),
    (md5('viewing_07')::uuid, md5('user_07')::uuid, md5('property_06')::uuid, '2026-04-02'),
    (md5('viewing_08')::uuid, md5('user_08')::uuid, md5('property_07')::uuid, '2026-04-06'),
    (md5('viewing_09')::uuid, md5('user_09')::uuid, md5('property_08')::uuid, '2026-04-07'),
    (md5('viewing_10')::uuid, md5('user_10')::uuid, md5('property_09')::uuid, '2026-04-08');
