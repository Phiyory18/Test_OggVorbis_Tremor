#include <iostream>

#define USE_OGGVORBIS_TREMOR

#ifndef NDEBUG
#pragma comment ( lib, "libogg_staticd.lib" )
#ifndef USE_OGGVORBIS_TREMOR
#pragma comment ( lib, "libvorbis_staticd.lib" )
#pragma comment ( lib, "libvorbisfile_staticd.lib" )
#else
#pragma comment (lib, "libtremord.lib")
#endif
#else
#pragma comment ( lib, "libogg_static.lib" )
#ifndef USE_OGGVORBIS_TREMOR
#pragma comment ( lib, "libvorbis_static.lib" )
#pragma comment ( lib, "libvorbisfile_static.lib" )
#else
#pragma comment (lib, "libtremor.lib")
#endif
#endif

#include "vorbis/ivorbisfile.h"

#include <windows.h>
#pragma comment ( lib, "winmm.lib" )

using namespace std;


/**
	WaveFileFormat�\����
*/
struct PCM_WAVE_FORMAT {
	char riff[4];
	unsigned int riffSize;
	char wave[4];
	char fmt[4];
	unsigned int fmtSize;
	unsigned short formatID;
	unsigned short channel;
	unsigned int samplingRate;
	unsigned int dataSizePerSec;
	unsigned short blockSize;
	unsigned short bitPerSample;
	char data[4];
	unsigned int PCMSize;

	// �R���X�g���N�^
	PCM_WAVE_FORMAT(unsigned short inChannel, unsigned int inSamplingRate, unsigned int inPCMDataSize) {
		// �Œ�l
		memcpy(riff, "RIFF", 4);
		memcpy(wave, "WAVE", 4);
		memcpy(fmt, "fmt ", 4);
		memcpy(data, "data", 4);
		fmtSize = 16;
		bitPerSample = 16; // 16bit�Œ�ɂ��܂�
		formatID = 1; // PCM

		// �l
		channel = inChannel;
		samplingRate = inSamplingRate;
		blockSize = bitPerSample / 8 * channel;
		dataSizePerSec = samplingRate * blockSize;

		// �T�C�Y�v�Z
		PCMSize = inPCMDataSize;
		riffSize = sizeof(PCM_WAVE_FORMAT) - 8 + PCMSize;
	}
};
char* createPCMWAVEFileImage( unsigned short channel, unsigned int   samplingRate, char* PCMData, unsigned int PCMDataSize) {
	unsigned int PCMImageSize = sizeof(PCM_WAVE_FORMAT) + PCMDataSize;
	char* PCMImage = new char[PCMImageSize];

	PCM_WAVE_FORMAT format(channel, samplingRate, PCMDataSize);
	memcpy(PCMImage, &format, sizeof(PCM_WAVE_FORMAT));
	memcpy(PCMImage + sizeof(PCM_WAVE_FORMAT), PCMData, PCMDataSize);

	return PCMImage;
}

const int PCMSize = 1024 * 10000; // 10MB

int main(int argc, char *argv[])
{
	cout << "Start..." << endl;

	OggVorbis_File ovf;
	FILE *pOggVorbisFile = nullptr;
	try{
		// File�I�[�v��
		FILE *pOggVorbisFile = fopen("bgm.ogg", "rb");
		if (!pOggVorbisFile) throw std::runtime_error("OGG�t�@�C�����J���܂���");

		// Ogg�I�[�v��
		if (ov_open(pOggVorbisFile, &ovf, NULL, 0) != 0) throw std::runtime_error("FAILURE => ov_open()");

		// Ogg�t�@�C���̉����t�H�[�}�b�g���
		vorbis_info* oggInfo = ov_info(&ovf, -1);

		// ���j�APCM�i�[
		char* buffer = new char[PCMSize];
		memset(buffer, 0, PCMSize);
		char* tmpBuffer = new char[PCMSize];
		int bitstream = 0;
		int readSize = 0;
		int comSize = 0;

		while (1) {
			// Ogg���[�h
			readSize = ov_read(&ovf, (char*)tmpBuffer, 4096, &bitstream);
			if (comSize + readSize >= PCMSize || readSize == EOF)
				break;
			memcpy(buffer + comSize, tmpBuffer, readSize);
			comSize += readSize;
		}
		delete[] tmpBuffer;

		// PCM + WaveHeader = WaveFile �쐬
		char *WaveImage = createPCMWAVEFileImage(oggInfo->channels, oggInfo->rate, buffer, comSize);

		// �Đ�
		PlaySound((LPCWSTR)WaveImage, NULL, SND_ASYNC | SND_MEMORY);

		Sleep(10000);

		// Ogg�N���A
		ov_clear(&ovf);

		// 
		delete[] WaveImage;
		delete[] buffer;

		cout << "End..." << endl;

	}
	catch (const std::exception& e){
		std::cout << e.what() << std::endl;
	}
		



	return 0;
}

