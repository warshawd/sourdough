#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

#define PROP_DELAY 20
#define LOWER_BOUND_SCALE 1.5
#define MEDIUM_BOUND_SCALE 5
#define UPPER_BOUND_SCALE 10

unsigned int the_window_size = 50;
unsigned int packets_this_cycle = 0;
/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  /* Default: fixed window size of 100 outstanding datagrams */

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
  // return 18;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp,
                                    /* in milliseconds */
				    const bool after_timeout
				    /* datagram was sent because of a timeout */ )
{
  if (after_timeout) {
    the_window_size = the_window_size / 2;
    if (the_window_size == 0) the_window_size = 1;
    packets_this_cycle = 0;
  }

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << " (timeout = " << after_timeout << ")\n";
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{

  uint64_t delay = timestamp_ack_received - send_timestamp_acked;
  if (delay > PROP_DELAY * MEDIUM_BOUND_SCALE) {
    if (the_window_size > 1) {
      the_window_size--;
    }
  }

  if (delay < PROP_DELAY * LOWER_BOUND_SCALE) {
    the_window_size ++;
  }
  packets_this_cycle++;
  if (packets_this_cycle >= the_window_size) {
      the_window_size += 2;
      packets_this_cycle = 0;
  }


  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return PROP_DELAY * UPPER_BOUND_SCALE;
}
