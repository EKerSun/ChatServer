
DROP TABLE IF EXISTS `allgroup`;
CREATE TABLE `allgroup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupname` varchar(50) CHARACTER SET latin1 NOT NULL,
  `groupdesc` varchar(200) CHARACTER SET latin1 DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `groupname` (`groupname`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci;



DROP TABLE IF EXISTS `friendships`;
CREATE TABLE `friendships` (
    `id` bigint NOT NULL AUTO_INCREMENT,
    `user_id1` bigint NOT NULL,
    `user_id2` bigint NOT NULL,
    `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `user_id1` (`user_id1`, `user_id2`),
    KEY `idx_user1` (`user_id1`),
    KEY `idx_user2` (`user_id2`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci;


DROP TABLE IF EXISTS `groupuser`;
CREATE TABLE `groupuser` (
  `groupid` int(11) NOT NULL,
  `userid` int(11) NOT NULL,
  `grouprole` enum('creator','normal') CHARACTER SET latin1 DEFAULT NULL,
  KEY `groupid` (`groupid`,`userid`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci;



DROP TABLE IF EXISTS `offlinemessage`;
CREATE TABLE `offlinemessage` (
  `userid` int(11) NOT NULL,
  `message` varchar(500) NOT NULL
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci;



DROP TABLE IF EXISTS `user`;
CREATE TABLE `user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) DEFAULT NULL,
  `password` varchar(50) DEFAULT NULL,
  `state` enum('online','offline') CHARACTER SET latin1 DEFAULT 'offline',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci;