CREATE SCHEMA `mmsdb` DEFAULT CHARACTER SET utf8 ;

CREATE  TABLE `mmsdb`.`mms_whitelist` (

  `idmms_whitelist` INT NOT NULL AUTO_INCREMENT ,

  `mms_whitelist_uid` INT UNSIGNED NOT NULL ,
   
  `mms_whitelist_user` VARCHAR(100) NULL ,

  `mms_whitelist_token` VARCHAR(128) NULL ,

  PRIMARY KEY (`idmms_whitelist`, `mms_whitelist_uid`) )

ENGINE = MyISAM

DEFAULT CHARACTER SET = utf8

COMMENT = 'white list used for login';

