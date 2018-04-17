#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* for estimating RRT */
uint64_t rrt_ms = 250;
double alpha = 0.8;
double beta = 2;
unsigned int the_window_size = 50;
uint64_t packets_since_increase = 0;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  /* Default: fixed window size of 100 outstanding datagrams */
  // unsigned int the_window_size = 50;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
   << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
            /* of the sent datagram */
            const uint64_t send_timestamp,
                                    /* in milliseconds */
            const bool after_timeout
            /* datagram was sent because of a timeout */ )
{
  /* Default: take no action */
  if (after_timeout) { //Multiplicative Decrease
    the_window_size = (the_window_size / 2);
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
  /* Default: take no action */
  const uint64_t cur_rrt_ms = timestamp_ack_received - send_timestamp_acked;
  rrt_ms = ((alpha * rrt_ms) + ((1-alpha) * (cur_rrt_ms)));

  if (the_window_size > packets_since_increase) { //Additive increase
    packets_since_increase += 1;
  } else {
    the_window_size += 1;
    packets_since_increase = 0;
  }

  // if (rrt_ms / 100 == 0) {
  //   cout << "wow rrt is " + rrt_ms << endl;
  // }

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
  return (beta * rrt_ms); 
}
