/* This program reads a message from stdin, detects its type and decodes it.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "unionproto.pb.h"
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define MAX_BUFFER 1500
#define SERVER "127.0.0.1"
#ifdef _WIN32
#define SOCKET_ERRNO	WSAGetLastError()
#else
#define SOCKET_ERRNO	errno
#endif
/* This function reads manually the first tag from the stream and finds the
* corresponding message type. It doesn't yet decode the actual message.
*
* Returns a pointer to the MsgType_fields array, as an identifier for the
* message type. Returns null if the tag is of unknown type or an error occurs.
*/
const pb_field_t* decode_unionmessage_type(pb_istream_t *stream)
{
	pb_wire_type_t wire_type;
	uint32_t tag;
	bool eof;

	while (pb_decode_tag(stream, &wire_type, &tag, &eof))
	{
		if (wire_type == PB_WT_STRING)
		{
			const pb_field_t *field;
			for (field = UnionMessage_fields; field->tag != 0; field++)
			{
				if (field->tag == tag && (field->type & PB_LTYPE_SUBMESSAGE))
				{
					/* Found our field. */
					return field->ptr;
				}
			}
		}

		/* Wasn't our field.. */
		pb_skip_field(stream, wire_type);
	}

	return NULL;
}

bool encode_unionmessage(pb_ostream_t *stream, const pb_field_t messagetype[], const void *message)
{
	const pb_field_t *field;
	for (field = UnionMessage_fields; field->tag != 0; field++)
	{
		if (field->ptr == messagetype)
		{
			/* This is our field, encode the message using it. */
			if (!pb_encode_tag_for_field(stream, field))
				return false;

			return pb_encode_submessage(stream, messagetype, message);
		}
	}

	/* Didn't find the field for messagetype */
	return false;
}
bool decode_unionmessage_contents(pb_istream_t *stream, const pb_field_t fields[], void *dest_struct)
{
	pb_istream_t substream;
	bool status;
	if (!pb_make_string_substream(stream, &substream))
		return false;

	status = pb_decode(&substream, fields, dest_struct);
	pb_close_string_substream(stream, &substream);
	return status;
}

bool encode_repeatedstring(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	char *str[4] = { "Hello world!", "", "Test", "Test2" };
	int i;

	for (i = 0; i < 4; i++)
	{
		if (!pb_encode_tag_for_field(stream, field))
			return false;

		if (!pb_encode_string(stream, (uint8_t*)str[i], strlen(str[i])))
			return false;
	}
	return true;
}

static int init_connection(void)
{
#ifdef _WIN32
	WSADATA WSAData;                    // Contains details of the 
										// Winsock implementation
										// Initialize Winsock. 
	if (WSAStartup(MAKEWORD(1, 1), &WSAData) != 0)
	{
		printf("WSAStartup failed! Error: %d\n", SOCKET_ERRNO);
		return FALSE;
	}
#endif
	/* UDP so SOCK_DGRAM */
	SOCKET sock;
	SOCKADDR_IN sin = { 0 };
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Allocating socket failed! Error: %d\n", SOCKET_ERRNO);
		return FALSE;
	}
	

	if (sock == INVALID_SOCKET)
	{
		perror("socket()");
		exit(errno);
	}

	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(PORT);
	sin.sin_family = AF_INET;

	if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
	{
		perror("bind()");
		exit(errno);
	}

	return sock;
}
static int read_client(SOCKET sock, SOCKADDR_IN *sin, char *buffer)
{
	int n = 0;
	size_t sinsize = sizeof *sin;

	if ((n = recvfrom(sock, buffer, MAX_BUFFER, 0, (SOCKADDR *)sin, &sinsize)) < 0)
	{
		perror("recvfrom()");
		exit(errno);
	}

	
	return n;
}

static int write_client(SOCKET sock, SOCKADDR_IN *sin, const uint8_t *buffer,const int length)
{
	int n = 0;
	if ((n = sendto(sock, buffer, length, 0, (SOCKADDR *)sin, sizeof *sin))< 0)
	{
		perror("send()");
		exit(errno);
	}
	return n;
}


int main()
{
	bool status = false;
	SOCKET sock = init_connection();
	uint8_t buffer[MAX_BUFFER];
	char s_name[5];
	while (1) /* Boucle infinie. Exercice : améliorez ce code. */
	{
		/* new client */
		SOCKADDR_IN csin = { 0 };
		int count = read_client(sock, &csin, s_name);
		printf("Got resquest.\n");
		pb_ostream_t output = pb_ostream_from_buffer(buffer, sizeof(buffer));
		MsgType1 msg1 = { 42 };
		MsgType2 msg2 = { true };
		MsgType3 msg3;
		msg3.playerName.funcs.encode = &encode_repeatedstring;
		status = encode_unionmessage(&output, MsgType3_fields, &msg3);
		bool sended = write_client(sock, &csin, buffer, output.bytes_written) == output.bytes_written;
		if (!sended)
			printf("Send error.\n");
	//printf("Closing connection.\n");	
	//closesocket(csock);
	}
	while(1)
	return 0;
}
