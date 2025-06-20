-- MySQL dump 10.13  Distrib 8.0.42, for Linux (aarch64)
--
-- Host: localhost    Database: talks
-- ------------------------------------------------------
-- Server version	8.0.42-0ubuntu0.22.04.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `chat`
--

DROP TABLE IF EXISTS `chat`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `chat` (
  `room_id` int NOT NULL,
  `user_id` varchar(16) NOT NULL,
  `chat_id` int NOT NULL,
  `chat_type` int NOT NULL DEFAULT '0',
  `text` varchar(64) DEFAULT NULL,
  `send_dt` datetime DEFAULT CURRENT_TIMESTAMP,
  `reply_chat_id` int DEFAULT '-1',
  `deleted` tinyint(1) DEFAULT (false),
  PRIMARY KEY (`room_id`,`user_id`,`chat_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `chat`
--

LOCK TABLES `chat` WRITE;
/*!40000 ALTER TABLE `chat` DISABLE KEYS */;
INSERT INTO `chat` VALUES (0,'\0',2,2,'e_giveup.png\0','2025-06-19 22:37:31',-1,0),(0,'\0',3,2,'e_nope.png\0','2025-06-19 22:37:33',-1,0),(0,'aaa\0',0,0,'dd\0','2025-06-19 22:05:18',-1,0),(0,'aaa\0',1,2,'e_nope.png\0','2025-06-19 22:05:20',-1,0),(0,'aaa\0',4,0,'dd\0','2025-06-19 22:37:46',-1,0),(0,'aaa\0',5,0,'11\0','2025-06-19 22:37:49',-1,0),(0,'aaa\0',6,0,'good\0','2025-06-19 22:38:08',3,0),(0,'aaa\0',7,0,'hello\0','2025-06-19 22:54:41',-1,0),(0,'aaa\0',8,0,'bye\0','2025-06-19 22:55:08',7,0),(0,'aaa\0',12,0,'go go\0','2025-06-19 23:04:11',-1,0),(0,'km1104rs\0',9,0,'hello\0','2025-06-19 22:58:45',-1,0),(0,'km1104rs\0',10,0,'d\0','2025-06-19 22:59:47',-1,0),(0,'km1104rs\0',11,0,'RECEIVE TEST\0','2025-06-19 23:00:18',-1,1),(1,'aaa\0',0,0,'hello\0','2025-06-19 22:52:07',-1,0);
/*!40000 ALTER TABLE `chat` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `chat_deleted`
--

DROP TABLE IF EXISTS `chat_deleted`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `chat_deleted` (
  `room_id` int NOT NULL,
  `user_id` varchar(16) NOT NULL,
  `chat_id` int NOT NULL,
  PRIMARY KEY (`room_id`,`user_id`,`chat_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `chat_deleted`
--

LOCK TABLES `chat_deleted` WRITE;
/*!40000 ALTER TABLE `chat_deleted` DISABLE KEYS */;
/*!40000 ALTER TABLE `chat_deleted` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `chat_react`
--

DROP TABLE IF EXISTS `chat_react`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `chat_react` (
  `user_id` varchar(16) NOT NULL,
  `room_id` int NOT NULL,
  `chat_id` int NOT NULL,
  `react_type` int NOT NULL,
  PRIMARY KEY (`room_id`,`user_id`,`chat_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `chat_react`
--

LOCK TABLES `chat_react` WRITE;
/*!40000 ALTER TABLE `chat_react` DISABLE KEYS */;
INSERT INTO `chat_react` VALUES ('aaa\0',0,4,4),('aaa\0',0,8,3);
/*!40000 ALTER TABLE `chat_react` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `refresh_tok`
--

DROP TABLE IF EXISTS `refresh_tok`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `refresh_tok` (
  `user_id` varchar(16) NOT NULL,
  `token` varchar(300) NOT NULL,
  `revoked` tinyint(1) DEFAULT (false),
  `exp_tm` int DEFAULT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `refresh_tok`
--

LOCK TABLES `refresh_tok` WRITE;
/*!40000 ALTER TABLE `refresh_tok` DISABLE KEYS */;
INSERT INTO `refresh_tok` VALUES ('aaa\0','eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3NTAzNzQ1NDMsImlzcyI6ImFkbWluX2NoYW4iLCJzdWIiOiJhYWEifQ.JSEiylFtVF42I8Fclk5wbdrvJvIQCfntmWfnwt7TuKw\0',0,1750374543),('km1104rs\0','eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3NTAzNzQwODIsImlzcyI6ImFkbWluX2NoYW4iLCJzdWIiOiJrbTExMDRycyJ9.dHnQFyFl1i5nStYC3pOpTJrBAIXbcPw4LL4HKXXOWt8\0',0,1750374082);
/*!40000 ALTER TABLE `refresh_tok` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `room`
--

DROP TABLE IF EXISTS `room`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `room` (
  `room_id` int NOT NULL,
  `room_name` varchar(16) NOT NULL,
  `create_dt` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`room_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `room`
--

LOCK TABLES `room` WRITE;
/*!40000 ALTER TABLE `room` DISABLE KEYS */;
INSERT INTO `room` VALUES (0,'TEST_01','2025-06-19 22:05:14'),(1,'TEST_01\0','2025-06-19 22:51:46');
/*!40000 ALTER TABLE `room` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `room_member`
--

DROP TABLE IF EXISTS `room_member`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `room_member` (
  `user_id` varchar(16) NOT NULL,
  `room_id` int NOT NULL,
  `last_read` int DEFAULT '0',
  PRIMARY KEY (`user_id`,`room_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `room_member`
--

LOCK TABLES `room_member` WRITE;
/*!40000 ALTER TABLE `room_member` DISABLE KEYS */;
INSERT INTO `room_member` VALUES ('aaa\0',0,13),('aaa\0',1,1),('bbb',0,0),('km1104rs',0,13);
/*!40000 ALTER TABLE `room_member` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user`
--

DROP TABLE IF EXISTS `user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `user` (
  `id` varchar(16) NOT NULL,
  `name` varchar(16) NOT NULL,
  `pwd` varchar(128) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user`
--

LOCK TABLES `user` WRITE;
/*!40000 ALTER TABLE `user` DISABLE KEYS */;
INSERT INTO `user` VALUES ('aa','123123','bb'),('aaa','ROBOT','111'),('admin','ADMIN','0000'),('km1104rs','hee chan','@gmlcks0915'),('km1104rs2\0\0\0\0\0\0\0','chan\0\0\0\0\0\0\0\0\0\0\0\0','1234\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0'),('km150rs','hee jun','1234'),('test2','TEST_ACCOUNT','1234');
/*!40000 ALTER TABLE `user` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2025-06-19 23:09:34
