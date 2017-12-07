/**
 * @file CRC.h
 * @date 14.08.2012
 * @author Eugen Paul, Mohamad Sbeiti
 */

#ifndef CRC_H_
#define CRC_H_

#include <stdlib.h>

class CRC {
private:
    /* crc_tab[] -- this crcTable is being build by chksum_crc32GenTab().
     *      so make sure, you call it before using the other
     *      functions!
     */
    u_int32_t crc_tab[256];
public:
    CRC();
    virtual ~CRC();

    /* chksum_crc() -- to a given block, this one calculates the
     *              crc32-checksum until the length is
     *              reached. the crc32-checksum will be
     *              the result.
     */
    u_int32_t chksum_crc32(unsigned char *block, unsigned int length);

    /* chksum_crc32gentab() -- to a global crc_tab[256], this one will
     *              calculate the crcTable for crc32-checksums.
     *              it is generated to the polynom [..]
     */

    void chksum_crc32gentab();

    bool checkCRC(u_int8_t* data, int length);

};

#endif /* CRC_H_ */
