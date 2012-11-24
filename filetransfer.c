//#include "filetransfer.h"
#include "tftp.h"

/* how use use these functions
   File handles should be opened and checked before any calls
  
   Client sending file:
    send WRQ
    wait for ack
    sendfile()
   Client recieving file:
    send RRQ
    recvfile()
   Server sending file:
    sendfile()
   Server receiving file:
    send ack
    recvfile()

 */

/* This function sends a file to a remote host client or server */
bool sendFile(int sockfd, struct sockaddr* cli_addr, FILE* fileh)
{
  //sudo-code
  //while there is data left to send:
  //  send a data packet
  //  wait for ack, check for error recieved
  //    if timeout:
  //      if timeout_counter > max_timeouts:
  //        send error
  //        return false
  //      else:
  //        increase timeout_counter
  //        resend data
  //    reset timeout_counter
  //return true

  u_int16_t blockNumber = 0;
  char buffer[MAX_DATA_SIZE];
  size_t n;

  do
  {
    n = fread(buffer,1,MAX_DATA_SIZE,fileh);
    send_data(sockfd, cli_addr, blockNumber,buffer,n);

    //wait for ack
    result = waitForPacket(sockfd, cli_addr, TFTP_OPTCODE_ACK,&packet);

    //check for timeout
    if (result == -1)
    {
      if (timeout_counter > MAX_TFTP_TIMEOUTS)
      {
        send_error(sockfd,cli_addr,TFTP_ERRCODE_ILLEGAL_OPERATION,"Too many timeouts");
        return false;
      }
      timeout_counter++;
    }else
    {
      //check for error
      if (packet->optcode == TFTP_OPTCODE_ERR)
      {
        //call the error handler
        printError(packet);
        return false;
      }
      //check the ack recieved was for the correct blcok number
      if (packet->ack.blockNumber != blockNumber)
      {
        send_error(sockfd,cli_addr,TFTP_ERRCODE_UNKNOWN_TRANSFER_ID,"Incorect Block Number");
        return false;
      }

      timeout_counter = 0;

      if (n > 0){
        //send the next chunk of data to the recivever
        n = fread(buffer,1,MAX_DATA_SIZE,fileh);
      }
    }
  } while (n == MAX_DATA_SIZE);

  return false;
}


/* This function recieves a file from a remote host client or server */
bool recvFile(int sockfd, struct sockaddr* cli_addr, FILE* fileh)
{
  
  PACKET packet;
  int result;
  unsigned int timeout_counter = 0;
  unsigned int expected_block_number = 1;

  do{
      result = waitForPacket(sockfd, cli_addr, TFTP_OPTCODE_DATA, &packet);
      if(result == -1)     //timeout
      {
        if(timeout_counter < MAX_TFTP_TIMEOUTS)
        {
          timeout_counter++;
        }
        else
        {
          send_error(sockfd, cli_addr, 0, "Too many timeouts.\n");
          return false; 
        }

      }
      else if(result == 0) //result == 0
      {
        if(packet.optcode == TFTP_OPTCODE_ERR)
        {
          //error handler
          return false;
        }

        //if not an error packet, return 
        return true;
      }
      else //correct packet
      {
        //check for correct blocknumber
        //A: Correct
        if(packet.data.blockNumber == expected_block_number)
        {
          if(fwrite(packet.data.data, 1, result, fileh))
          {
            expected_block_number++;
            send_ack(sockfd, cli_addr, packet.data.blockNumber);
          }
          else 
          {
            send_error(sockfd, cli_addr, 0, "Did not write to file.\n");
            return false;
          }
        }
        else
        {
            send_error(sockfd, cli_addr, 0, "Incorrect packet recieved.\n")
        }
      } 
  } while(result == MAX_DATA_SIZE);

  return true;
}
