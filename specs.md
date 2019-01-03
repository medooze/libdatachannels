Datagram Transport Layer Security (DTLS) Encapsulation of SCTP Packets
https://tools.ietf.org/html/rfc8261

WebRTC Data Channels
https://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-13


SCTP as specified in [RFC4960] MUST be used in combination with the extension defined in [RFC3758]

   - The encapsulation of SCTP over DTLS defined in   [RFC8261](https://tools.ietf.org/html/rfc8261)

   - No MTU discovery: The initial Path MTU at the IP layer SHOULD NOT exceed 1200 bytes.
   
   Each SCTP user message contains a Payload Protocol Identifier (PPID)
   that is passed to SCTP by its upper layer on the sending side and
   provided to its upper layer on the receiving side.  The PPID can be
   used to multiplex/demultiplex multiple upper layers over a single
   SCTP association.  In the WebRTC context, the PPID is used to
   distinguish between UTF-8 encoded user data, binary encoded userdata
   and the Data Channel Establishment Protocol defined in [I-D.ietf-rtcweb-data-protocol](https://tools.ietf.org/html/draft-ietf-rtcweb-data-protocol-09)


The following SCTP protocol extensions are required:

   -  The stream reconfiguration extension defined in [RFC6525] MUST be
      supported.  It is used for closing channels.

   -  The dynamic address reconfiguration extension defined in [RFC5061]
      MUST be used to signal the support of the stream reset extension
      defined in [RFC6525].  Other features of [RFC5061] are OPTIONAL.

   -  The partial reliability extension defined in [RFC3758] MUST be
      supported.  In addition to the timed reliability PR-SCTP policy
      defined in [RFC3758], the limited retransmission policy defined in
      [I-D.ietf-tsvwg-sctp-prpolicies] MUST be supported.  Limiting the
      number of retransmissions to zero combined with unordered delivery
      provides a UDP-like service where each user message is sent
      exactly once and delivered in the order received.

   - The support for message interleaving as defined in [I-D.ietf-tsvwg-sctp-ndata] SHOULD be used.  


   
