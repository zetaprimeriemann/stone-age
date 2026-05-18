#define _SASQL_C_

#include "version.h"

#ifdef _SASQL			//������

#include "main.h"
#include "sasql.h"
#include "util.h"

#include <mysql/mysql.h>

MYSQL mysql;
MYSQL_RES *mysql_result;
MYSQL_ROW mysql_row;

typedef struct tagConfig {
	char sql_IP[16];

	int sql_Port;

	char sql_Port1[16];

	char sql_ID[16];

	char sql_PS[16];

	char sql_DataBase[16];

	char sql_Table[16];

	char sql_LOCK[16];

	char sql_NAME[16];

	char sql_PASS[16];
} Config;

Config config;

int AutoReg;

static int readConfig(char *path)
{
	char buf[255];
	FILE *fp;

	fp = fopen(path, "r");
	if (fp == NULL) {
		return -2;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		char command[255];
		char param[255];
		chop(buf);

		easyGetTokenFromString(buf, 1, command, sizeof(command));
		easyGetTokenFromString(buf, 2, param, sizeof(param));

		if (strcmp(command, "sql_IP") == 0) {
			strcmp(config.sql_IP, param);
			snprintf(config.sql_IP, sizeof(config.sql_IP), param);
			printf("\n���ݿ��ַ��  %s", config.sql_IP);
		} else if (strcmp(command, "sql_Port") == 0) {
			config.sql_Port = atoi(param);
			snprintf(config.sql_Port1, sizeof(config.sql_Port1),
				 param);
			printf("\n���ݿ�˿ڣ�  %d", config.sql_Port);
		} else if (strcmp(command, "sql_ID") == 0) {
			strcmp(config.sql_ID, param);
			snprintf(config.sql_ID, sizeof(config.sql_ID), param);
			printf("\n���ݿ��û���  %s", config.sql_ID);
		} else if (strcmp(command, "sql_PS") == 0) {
			strcmp(config.sql_PS, param);
			snprintf(config.sql_PS, sizeof(config.sql_PS), param);
			printf("\n���ݿ����룺  %s", config.sql_PS);
		} else if (strcmp(command, "sql_DataBase") == 0) {
			strcmp(config.sql_DataBase, param);
			snprintf(config.sql_DataBase,
				 sizeof(config.sql_DataBase), param);
			printf("\n��½���ݿ�����%s", config.sql_DataBase);
		} else if (strcmp(command, "sql_Table") == 0) {
			strcmp(config.sql_Table, param);
			snprintf(config.sql_Table, sizeof(config.sql_Table),
				 param);
			printf("\n�û���Ϣ������  %s", config.sql_Table);
		} else if (strcmp(command, "sql_LOCK") == 0) {
			strcmp(config.sql_LOCK, param);
			snprintf(config.sql_LOCK, sizeof(config.sql_LOCK),
				 param);
			printf("\n�û�����������  %s", config.sql_LOCK);
		} else if (strcmp(command, "sql_NAME") == 0) {
			strcmp(config.sql_NAME, param);
			snprintf(config.sql_NAME, sizeof(config.sql_NAME),
				 param);
			printf("\n�˺��ֶ����ƣ�  %s", config.sql_NAME);
		} else if (strcmp(command, "sql_PASS") == 0) {
			strcmp(config.sql_PASS, param);
			snprintf(config.sql_PASS, sizeof(config.sql_PASS),
				 param);
			printf("\n�����ֶ����ƣ�  %s", config.sql_PASS);
		} else if (strcmp(command, "AutoReg") == 0) {
			AutoReg = atoi(param);
			if (AutoReg) {
				printf("\n�����Զ�ע�᣺YES");
			} else {
				printf("\n�����Զ�ע�᣺NO");
			}
			fclose(fp);
			return 0;
		}
	}
}

BOOL sasql_init(void)
{
	if (mysql_init(&mysql) == NULL & readConfig("acserv.cf")) {
		printf("\nmysql_init=fail ���ݿ��ʼ��ʧ�ܣ�");
		exit(1);
		return FALSE;
	}
	printf("ip=%s id=%s ps=%s database=%s port=%d",
	       config.sql_IP, config.sql_ID, config.sql_PS, config.sql_DataBase,
	       config.sql_Port);

	if (!mysql_real_connect(&mysql, config.sql_IP, config.sql_ID,	//�ʺ�
				config.sql_PS,	//����
				config.sql_DataBase,	//ѡ������Ͽ�
				config.sql_Port, NULL, 0)) {
		printf("\nmysql_real_connect=fail ���ݿ�����ʧ�ܣ�\n");
		return FALSE;
	}

	printf("\nmysql_real_connect=ok ���ݿ����ӳɹ���\n");
	return TRUE;
}

void sasql_close(void)
{
	mysql_close(&mysql);
}

int sasql_query(char *nm, char *pas)
{
	char sqlstr[256];
	sprintf(sqlstr, "select * from %s where %s=BINARY'%s'",
		config.sql_Table, config.sql_NAME, nm);
	printf("\nquery_sql=%s\n", sqlstr);
	if (!mysql_query(&mysql, sqlstr)) {
		int num_row = 0;
		mysql_result = mysql_store_result(&mysql);
		num_row = mysql_num_rows(mysql_result);
		if (num_row > 0) {
			mysql_row = mysql_fetch_row(mysql_result);
			if (strcmp(pas, mysql_row[1]) == 0) {
		        mysql_free_result(mysql_result);
				return 1;
			} else {
				printf
				    ("password=not_correct �û�%s�������\n",
				     nm);
		        mysql_free_result(mysql_result);
				return 2;
			}
		} 
		mysql_free_result(mysql_result);
		printf("user=not_register �û�%sδע�ᣡ\n", nm);
		return 3;
	} else {
		printf("\nmysql_error=%s\n", mysql_error(&mysql));
		printf("\nmysql_query=fail ���ݿ����ʧ�ܣ�\n");
		printf("\nreconnect_db=yes �����������ݿ�...\n");
		sasql_close();
		sasql_init();
		printf("finish ���\n");
		return 0;
	}
}

#ifdef _SQL_REGISTER
BOOL sasql_register(char *id, char *ps)
{
	char sqlstr[256];
//      if(AutoReg!=1)return FALSE;
	sprintf(sqlstr,
		"INSERT INTO %s (%s,%s,RegTime,Path) VALUES (BINARY'%s','%s',NOW(),'char/0x%x')",
		config.sql_Table, config.sql_NAME, config.sql_PASS, id, ps,
		getHash(id) & 0xff);
	printf("\nregister_sql=%s\n", sqlstr);
	if (!mysql_query(&mysql, sqlstr)) {
		printf("\nnew_user_register=ok ���û�ע��ɹ���\n");
		return TRUE;
	}
	printf("\nnew_user_register=fail ���û�ע��ʧ�ܣ�\n");
	return FALSE;
}
#endif

BOOL sasql_chehk_lock(char *idip)
{
	char sqlstr[256];
	sprintf(sqlstr, "select * from %s where %s=BINARY'%s'", config.sql_LOCK,
		config.sql_NAME, idip);
	printf("\ncheck_lock_sql=%s\n", sqlstr);

	/* TODO: check the lock according to result's rows */

	if (!mysql_query(&mysql, sqlstr)) {
		int num_row = 0;
		mysql_result = mysql_store_result(&mysql);
		num_row = mysql_num_rows(mysql_result);
		mysql_free_result(mysql_result);
		if (num_row > 0) {
			return TRUE;	/* account is locked */
		}
	}

	return FALSE;
}

BOOL sasql_add_lock(char *idip)
{
	char sqlstr[256];
	sprintf(sqlstr, "INSERT INTO %s (%s) VALUES (BINARY'%s')",
		config.sql_LOCK, config.sql_NAME, idip);
	printf("\nadd_lock_sql=%s\n", sqlstr);
	if (!mysql_query(&mysql, sqlstr)) {
		printf("\n��������%s�ɹ���\n", idip);
		return TRUE;
	}
	return FALSE;
}

BOOL sasql_del_lock(char *idip)
{
	char sqlstr[256];
	sprintf(sqlstr, "delete from config.SQL_LOCK where %s=BINARY'%s'",
		config.sql_LOCK, config.sql_NAME, idip);
	printf("\ndel_lock_sql=%s\n", sqlstr);
	if (!mysql_query(&mysql, sqlstr)) {
		printf("\n�������%s�ɹ���\n", idip);
		return TRUE;
	}
	return FALSE;
}

BOOL sasql_craete_lock(void)
{

}

BOOL sasql_craete_userinfo(void)
{

}

#endif
