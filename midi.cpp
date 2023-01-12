#include <cstdint>
#include "midi.h"

namespace su_midi{
    midi_receiver_base::midi_receiver_base():midi_receiver_base(ALL_CHANNEL_MASK){
   
    }
    midi_receiver_base::midi_receiver_base(std::uint16_t mask){
        state = INIT_STATE;
        status_byte = 0x00;
        first_arg = 0x00;
        channel_mask = mask;             
    }
    void midi_receiver_base::parse_byte(std::uint8_t dat){
        if(state == INIT_STATE){
            if((dat & MSG_STATUS_MASK) == MSG_STATUS_MASK){//If it is a status byte
                if((dat & MSG_SYSTEM_REALTIME_MASK) == MSG_SYSTEM_REALTIME_MASK){//And if it's a realtime msg
                    if(dat == MSG_TIMING_CLOCK){
                        timing_clock_handler();
                    }
                    else if(dat == MSG_START){
                        start_handler();
                    }
                    else if(dat == MSG_CONTINUE){
                        continue_handler();
                    }
                    else if(dat == MSG_STOP){
                        stop_handler();
                    }
                    else if(dat == MSG_ACTIVE_SENSING){
                        active_sensing_handler();
                    }
                    else if(dat == MSG_SYSTEM_RESET){
                        system_reset_handler();
                    }
                }
                else{
                    //In case of unintended end of exclusive by recieving another message except realtime message.
                    if(status_byte == MSG_SYSEX_START){
                        end_of_exclusive_handler();
                    }

                    status_byte = dat;//set status byte
                    recieve_ch = dat & MSG_CH_MASK;
                    if((status_byte & MSG_TYPE_MASK) != 0xf0)status_byte &= MSG_TYPE_MASK;
                    
                    if(status_byte == MSG_TUNE_REQ){
                        tune_request_handler();
                    }
                    else if(status_byte == MSG_SYSEX_START){
                        start_of_exclusive_handler();
                    }
                    else if(status_byte == MSG_SYSEX_END){
                        end_of_exclusive_handler();
                    }
                }
            }
            else{//Then it is 1st arg. of msg.
                first_arg = dat;
                if(status_byte == MSG_PC){
                    if((channel_mask & (1 << recieve_ch)) != 0){
                        pc_handler(first_arg,recieve_ch);
                    }
                }
                else if(status_byte == MSG_CH_PRESSURE){
                    if((channel_mask & (1 << recieve_ch)) != 0){
                        ch_pressure_handler(first_arg,recieve_ch);
                    }
                }
                else if(status_byte == MSG_QUARTER_FRAME){
                    quarter_frame_handler(first_arg);
                }
                else if(status_byte == MSG_SONG_SELECT){
                    song_select_handler(first_arg);
                }
                else if(status_byte == MSG_SYSEX_START){
                    //Data of SYSEX Message
                    data_of_exclusive_handler(first_arg);
                }
                else{//Msg that have 2 args
                    state = WAIT_2ND_ARG;
                }
            }
        }
        else if(state == WAIT_2ND_ARG){
            if(status_byte == MSG_NOTE_OFF){
                if((channel_mask & (1 << recieve_ch)) != 0){
                    note_off_handler(first_arg,dat,recieve_ch);
                }
            }
            else if(status_byte == MSG_NOTE_ON){
                if((channel_mask & (1 << recieve_ch)) != 0){
                    note_on_handler(first_arg,dat,recieve_ch);
                }
            }
            else if(status_byte == MSG_POLY_PRESSURE){
                if((channel_mask & (1 << recieve_ch)) != 0){
                    poly_pressure_handler(first_arg,dat,recieve_ch);
                }
            }
            else if(status_byte == MSG_CC){
                if((channel_mask & (1 << recieve_ch)) != 0){
                    cc_handler(first_arg,dat,recieve_ch);
                }
            }
            else if(status_byte == MSG_PITCH_BEND){
                std::uint16_t tmp;
                tmp = first_arg | (dat << 7);
                if((channel_mask & (1 << recieve_ch)) != 0){
                    pitchbend_handler(tmp - 8192,recieve_ch);
                }
            }
            else if(status_byte == MSG_SONG_PTR){
                std::uint16_t tmp;
                tmp = first_arg | (dat << 7);
                song_pointer_handler(tmp);
            }
            state = INIT_STATE;
        }
    }
    void midi_receiver_base::set_channel_mask(std::uint16_t mask){
        channel_mask = mask;
    }
}