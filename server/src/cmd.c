#include "cmd.h"
#include "stdio.h"
#include "string.h"

int parse_cmd(uint8_t * buffer, uint8_t len)
{
    int rv = 0;
    if (buffer[0]==CMD_GO)
    {
        switch(buffer[1])
        {
            case (GO_STR):
            printf("GO_STR");
            break;
            case (GO_BACK):
            printf("GO_BACK");
            break;
            case (GO_LEFT):
            printf("GO_LEFT");
            break;
            case (GO_RIGHT):
            printf("GO_RIGHT");
            break;
            case (GO_STR_LEFT):
            printf("GO_STR_LEFT");
            break;
            case (GO_BACK_RIGHT):
            printf("GO_BACK_RIGHT");
            break;
            case (GO_BACK_LEFT):
            printf("GO_BACK_LEFT");
            break;
            case (GO_STR_RIGHT):
            printf("GO_STR_RIGHT");
            break;
            case (GO_STOP):
            printf("GO_STOP");
            break;
            default:
            printf("Invalid cmd");
            rv = -1;
            break; 
        }
        printf("\n");
    }
    else rv = -1;
    //c_buff->buff_len-= len_cmd;
    //memcpy(c_buff->buffer, &c_buff->buffer[c_buff->LenToDo[c_buff->pos]],c_buff->LenToDo[c_buff->pos]);
    //c_buff->pos++;
    return rv;
    
}