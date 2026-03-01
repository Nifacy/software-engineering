workspace "Property Management System" "Система для управления недвижимостью" {
    !identifiers hierarchical

    model {
        addressParserSystem = softwareSystem "Address Parser" {
            description "Система для парсинга и валидации адресов"
            tags "system"
        }

        propertyHubSystem = softwareSystem "Property Hub" {
            description "Система для управления недвижимостью"
            tags "system"

            dataStorage = container "Data Storage" {
                description "Хранение и обновление данных"
                tags "storage"
                technology "PostgreSQL"
            }

            searchEngine = container "Search Engine" {
                description "Поисковый движок, управление поисковым индексом"
                technology "Meilisearch"
            }

            accountSearchIndex = container "Account Search Index" {
                description "Поиск аккаунтов пользователей по критериям"
                technology "Python, faststream"

                -> searchEngine "Обновление индекса пользователей" "HTTP, REST"
            }

            propertySearchIndex = container "Property Search Index" {
                description "Поиск объектов недвижимости по критериям"
                technology "Python, faststream"

                -> searchEngine "Обновление индекса объектов" "HTTP, REST"
            }

            authService = container "Auth Service" {
                description "Авторизация пользователей"
                technology "Python, faststream, JWT"

                -> dataStorage "Получение и обновление данных для входа пользователя" "JDBC"
            }

            visitScheduleService = container "Visit Schedule Service" {
                description "Управление расписанием на осмотр объектов недвижимости"
                technology "Python, faststream"

                -> dataStorage "Получение и обновление данных о расписании на осмотр объекта" "JDBC"
            }

            propertyService = container "Property Service" {
                description "Управление объектами недвижимости"
                technology "Python, faststream"

                -> dataStorage "Получение и обновление данных об объекте" "JDBC"
                -> visitScheduleService "Создание, удаление списка на осмотр, соответствующего объекту" "AMQP"
                -> addressParserSystem "Отправка адресов, указанных пользователем, для валидации и парсинга"
                -> propertySearchIndex "Обновление индекса объекта" "AMQP"
            }

            accountService = container "Account Service" {
                description "Управление пользователями и их личными данными"
                technology "Python, faststream"

                -> authService "Аутентификация, регистрация пользователя" "AMQP"
                -> accountSearchIndex "Обновление индекса аккаунта" "AMQP"
                -> visitScheduleService "Запись, снятие себя с записи на осмотр" "AMQP"
                -> dataStorage "Получение и обновление данных об аккаунте" "JDBC"
                -> propertyService "Создание, удаление, обновление статуса объекта недвижимости" "AMQP"
            }

            apiGateway = container "API Gateway" {
                description "Публичный API системы"
                technology "Python, FastAPI"

                -> accountService "Маршрутизация запросов, связанных с пользователем"
                -> visitScheduleService "Маршрутизация запросов, связанных со списком на осмотр"
                -> propertyService "Маршрутизация запросов, связанных с объектом недвижимости"
                -> accountSearchIndex "Маршрутизация запросов, связанных с поиском пользователей"
                -> propertySearchIndex "Маршрутизация запросов, связанных с поиском объектов"
                -> authService "Авторизация пользователя"
            }
        }

        user = person "Пользователь" "Конечный пользователь системы" {
            -> propertyHubSystem.apiGateway "Создание аккаунта"
            -> propertyHubSystem.apiGateway "Поиск пользователей"
            -> propertyHubSystem.apiGateway "Создание объекта недвижимости"
            -> propertyHubSystem.apiGateway "Изменение статуса объекта недвижимости"
            -> propertyHubSystem.apiGateway "Поиск объектов недвижимости"
            -> propertyHubSystem.apiGateway "Получение информации об объекте недвижимости"
        }

    }

    views {
        themes default

        systemContext propertyHubSystem "SystemContextDiagram" {
            include *
            autoLayout
        }

        container propertyHubSystem "ContainerContextDiagram" {
            include *
            autoLayout
        }

        dynamic propertyHubSystem "CreateProperty" "Добавление объекта недвижимости" {
            autoLayout

            user -> propertyHubSystem.apiGateway "POST /api/v1/account/property"

                propertyHubSystem.apiGateway -> propertyHubSystem.accountService "Создание нового объекта недвижимости"

                    propertyHubSystem.accountService -> propertyHubSystem.authService "Аутентификация пользователя"
                    propertyHubSystem.authService -> propertyHubSystem.accountService "Возврат результата аутентификации"

                    propertyHubSystem.accountService -> propertyHubSystem.propertyService "Создание нового объекта недвижимости"

                        propertyHubSystem.propertyService -> addressParserSystem "Парсинг введенного адреса объекта"
                        addressParserSystem -> propertyHubSystem.propertyService "Возврат результата парсинга"

                        propertyHubSystem.propertyService -> propertyHubSystem.visitScheduleService "Созданиие расписания на осмотр объекта"
                        propertyHubSystem.visitScheduleService -> propertyHubSystem.propertyService "ID созданного расписания"

                        propertyHubSystem.propertyService -> propertyHubSystem.dataStorage "Создание записи о новом объекте"
                        propertyHubSystem.dataStorage -> propertyHubSystem.propertyService "Возврат результата добавления"

                        propertyHubSystem.propertyService -> propertyHubSystem.propertySearchIndex "Обновление индекса объектов"

                            propertyHubSystem.propertySearchIndex -> propertyHubSystem.searchEngine "Обновление индекса объектов"
                            propertyHubSystem.searchEngine -> propertyHubSystem.propertySearchIndex "Возврат результата обновления индекса"

                        propertyHubSystem.propertySearchIndex -> propertyHubSystem.propertyService "Возврат результата обновления индекса"

                    propertyHubSystem.propertyService -> propertyHubSystem.accountService "Возврат результата создания нового объекта"

                    propertyHubSystem.accountService -> propertyHubSystem.dataStorage "Добавление нового объекта в список недвижимости аккаунта"
                    propertyHubSystem.dataStorage -> propertyHubSystem.accountService "Возврат результата добавления"

                propertyHubSystem.accountService -> propertyHubSystem.apiGateway "Возврат результата добавления объекта"

            propertyHubSystem.apiGateway -> user "Возврат результата добавления"
        }
        
        styles {
            element "storage" {
                shape Cylinder
                width 300
                background #ababab
                stroke #686868
                strokeWidth 4
            }
            
            element "system" {
                shape Hexagon
                background #ababab
                stroke #686868
                strokeWidth 4
            }

            element "Person" {
                shape Person
                background #ababab
                stroke #686868
                strokeWidth 8
            }
        }
    }
}
