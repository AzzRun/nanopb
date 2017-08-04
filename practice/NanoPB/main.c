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

bool print_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
	uint8_t buffer[1024] = { 0 };

	/* We could read block-by-block to avoid the large buffer... */
	if (stream->bytes_left > sizeof(buffer) - 1)
		return false;

	if (!pb_read(stream, buffer, stream->bytes_left))
		return false;

	/* Print the string, in format comparable with protoc --decode.
	* Format comes from the arg defined in main().
	*/
	printf((char*)*arg, buffer);
	return true;
}

bool encode_repeatedstring(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	//char *str[6] = { "Hello world!", "Hi", "Bonjour", "Salut" , "Bonjour", "Salut" };
	char str[7] = "coucou\0";

	/*for (i = 0; i < 6; i++)
	{*/
	if (!pb_encode_tag_for_field(stream, field))
		return false;

	if (!pb_encode_string(stream, (uint8_t*)str, strlen(str)))
		return false;
	//}
	return true;
}
static int init_connection(const char *address, SOCKADDR_IN *sin)
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
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct hostent *hostinfo;

	if (sock == INVALID_SOCKET)
	{
		perror("socket()");
		exit(errno);
	}

	hostinfo = gethostbyname(address);
	if (hostinfo == NULL)
	{
		fprintf(stderr, "Unknown host %s.\n", address);
		exit(EXIT_FAILURE);
	}

	sin->sin_addr = *(IN_ADDR *)hostinfo->h_addr;
	sin->sin_port = htons(PORT);
	sin->sin_family = AF_INET;

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

static int write_client(SOCKET sock, SOCKADDR_IN *sin, const char *buffer, const int length)
{
	int n = 0;
	if ((n = sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)sin, sizeof *sin))< 0)
	{
		perror("send()");
		exit(errno);
	}
	return 0;
}

int main()
{
	bool status = false;
	char s_name[5] = "jack";
	SOCKADDR_IN sin = { 0 };
	SOCKET sock = init_connection(SERVER, &sin);
	uint8_t buffer[MAX_BUFFER];
	write_client(sock, &sin, s_name,strlen(s_name));

	int count = read_client(sock, &sin, buffer);
	pb_istream_t stream = pb_istream_from_buffer(buffer, count);
	const pb_field_t *type = decode_unionmessage_type(&stream);

	if (type == MsgType1_fields)
	{
		MsgType1 msg;
		status = decode_unionmessage_contents(&stream, MsgType1_fields, &msg);
		printf("Got MsgType1: %d\n", msg.value);
	}
	else if (type == MsgType2_fields)
	{
		MsgType2 msg;
		status = decode_unionmessage_contents(&stream, MsgType2_fields, &msg);
		printf("Got MsgType2: %s\n", msg.value ? "true" : "false");
	}
	else if (type == MsgType3_fields)
	{
		MsgType3 msg;
		printf("Got MsgType3");
		msg.playerName.funcs.decode = &print_string;
		msg.playerName.arg = "repeatedstring: \"%s\"\n";
		status = decode_unionmessage_contents(&stream, MsgType3_fields, &msg);
	}

		
	while (true)
	{
	}
		
	return 0;
}
