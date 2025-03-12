


CREATE TABLE `friendships` (
    `id` bigint NOT NULL AUTO_INCREMENT,
    `user_id1` bigint NOT NULL,
    `user_id2` bigint NOT NULL,
    `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `user_id1` (`user_id1`, `user_id2`),
    KEY `idx_user1` (`user_id1`),
    KEY `idx_user2` (`user_id2`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci


CREATE TABLE `user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) DEFAULT NULL,
  `password` varchar(50) DEFAULT NULL,
  `state` enum('online','offline') CHARACTER SET latin1 DEFAULT 'offline',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci;