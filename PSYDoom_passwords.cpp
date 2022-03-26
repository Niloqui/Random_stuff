#include <iostream>

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom helper to make the encoding logic byte bit cleaner and remove some redundancy.
// Divide 'byte' by 'b' and do byte 'ceil' operation on the potentially non integer result.
// Returns the answer as an unsigned 8-bit integer.
//------------------------------------------------------------------------------------------------------------------------------------------
static inline uint8_t ceil8Div(const int32_t num, const int32_t den) noexcept {
    const int32_t result = num / den + ((num % den != 0) ? 1 : 0);
    return (uint8_t)result;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Compute a password for the current player and save in the given output buffer.
// Each byte in the output buffer contains a 5-bit number from 0-31, which corresponds to one of the allowed password chars/digits.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_ComputePassword(uint8_t pOutput[10], uint8_t map, uint8_t skill) noexcept {
    // Get the player to encode the password for and zero init the unencrypted password

    uint8_t pwdata[8];
    //D_memset(pwdata, std::byte(0), 8);

    for (int i = 0; i < 8; i++) {
        pwdata[i] = 0;
    }

    // Pistol-start stats
    const bool bPlayerBackpack = false;
    const int32_t playerClips = 50;
    const int32_t playerShells = 0;
    const int32_t playerCells = 0;
    const int32_t playerMissiles = 0;
    const int32_t playerHealth = 100;
    const int32_t playerArmorPoints = 0;
    const int32_t playerArmorType = 0;

    // Encode byte: current map and skill
    pwdata[0] = (uint8_t)((map & 63) << 2);
    pwdata[0] |= (uint8_t)(skill & 3);


    pwdata[1] |= (bPlayerBackpack) ? 0x80 : 0;

    // Determine the maximum ammo amount for the calculations below
    const uint8_t maxAmmoShift = (bPlayerBackpack) ? 1 : 0;

    const int32_t maxClips = 200;
    const int32_t maxShells = 50;
    const int32_t maxCells = 300;
    const int32_t maxMissiles = 50;

    // Encode byte: number of bullets and shells (in 1/8 of the maximum increments, rounded up)
    const uint8_t clipsEnc = ceil8Div(playerClips << 3, maxClips);
    const uint8_t shellsEnc = ceil8Div(playerShells << 3, maxShells);
    pwdata[2] = (clipsEnc << 4) | shellsEnc;

    // Encode byte: number of cells and missiles (in 1/8 of the maximum increments, rounded up)
    const uint8_t cellsEnc = ceil8Div(playerCells << 3, maxCells);
    const uint8_t missilesEnc = ceil8Div(playerMissiles << 3, maxMissiles);
    pwdata[3] = (cellsEnc << 4) | missilesEnc;

    // Encode byte: health and armor points (in 1/8 of the maximum increments (25 HP), rounded up)
    const uint8_t healthPointsEnc = ceil8Div(playerHealth << 3, 200);
    const uint8_t armorPointsEnc = ceil8Div(playerArmorPoints << 3, 200);
    pwdata[4] = (healthPointsEnc << 4) | armorPointsEnc;

    // Encode byte: armor type
    pwdata[5] = (uint8_t)(playerArmorType << 3);

    // PsyDoom: incorporating an improvement from PSXDOOM-RE to encode an additional 2 map number bits.
    // This allows for map numbers from 0-255 instead of just 0-63, if required.
    pwdata[5] |= (map & 64) ? 0x20 : 0;
    pwdata[5] |= (map & 128) ? 0x40 : 0;

    // PsyDoom: encode if the game is operating in nightmare mode in the top bit of the last unencrypted byte.
    // This change is compatible with a similar change in 'PSXDOOM-RE' so passwords should be compatible beween both projects.
    //
    // Note: only the top 5 bits of the last unencrypted byte are encoded to a password. 2 bits are used by armortype, and now
    // 1 extra bit is used by nightmare mode. Therefore there are still 2 bits left for over purposes, perhaps extended level support?
//#if PSYDOOM_MODS
    //if (skill == sk_nightmare) {
    if (skill == 4) {
        pwdata[5] |= 0x80;
    }
    //#endif

        // Convert the regular 8-bit bytes that we just encoded to 5-bit bytes which can encode 32 values.
        // This is so we can use ASCII characters and numbers to encode the data.
        // This expands the size of the password from 6 bytes to 9 bytes, as we need to encode 45 bits.
    constexpr int32_t BITS_TO_ENCODE = 45;

    for (int32_t srcBitIdx = 0; srcBitIdx < BITS_TO_ENCODE;) {
        // Encode 5 source bits and save the 5-bit byte
        uint8_t dstByte = 0;

        for (int32_t bitInDstByte = 4; bitInDstByte >= 0; --bitInDstByte, ++srcBitIdx) {
            const uint8_t srcByte = pwdata[srcBitIdx / 8];
            const uint8_t srcBitMask = (uint8_t)(0x80u >> (srcBitIdx & 7));
            const uint8_t dstBitMask = (uint8_t)(1u << bitInDstByte);

            if (srcByte & srcBitMask) {
                dstByte |= dstBitMask;      // Encode the source bit when set
            }
        }

        const int32_t dstByteIdx = (srcBitIdx - 1) / 5;     // -1 because we are now on the next dest byte
        pOutput[dstByteIdx] = dstByte;
    }

    // Simple encryption: build an XOR bitmask to apply to the 9 data bytes, from XOR-ing all 9 data bytes.
    // This 5-bit XOR pattern is stored in the last output byte of the password.
    pOutput[9] = 0;

    for (int32_t i = 0; i < 9; ++i) {
        pOutput[9] ^= pOutput[i];
    }

    // Now apply the XOR pattern to encrypt the 9 data bytes
    for (int32_t i = 0; i < 9; ++i) {
        pOutput[i] ^= pOutput[9];
    }
}



int main() {
    char difficultes[5][20] = { "IAAW", "NTR", "HMP", "UV", "NM" };
    char letters[33] = "BCDFGHJKLMNPQRSTVWXYZ0123456789!";
    uint8_t password[10];

    for (int i = 0; i < 64; i++) {

        if (i == 0) // First row
            std::cout << "Levels\t";
        else
            std::cout << "Map" << i << "\t";


        for (int j = 0; j < 5; j++) {

            if (i == 0) { // First row
                std::cout << difficultes[j] << "\t\t";
            }
            else {
                P_ComputePassword(password, i, j);

                for (int k = 0; k < 10; k++) {
                    std::cout << letters[password[k]];
                    //std::cout << (int)password[k] << ".";
                }

                std::cout << "\t";
            }
        }

        std::cout << "\n";
    }


    system("PAUSE");
}


