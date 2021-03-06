
/*
 * CINELERRA
 * Copyright (C) 2008 Adam Williams <broadcast at earthling dot net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

#include "audioconfig.h"
#include "audiodevice.h"
#include "audiooss.h"
#include "clip.h"
#include "condition.h"
#include "errno.h"
#include "playbackconfig.h"
#include "preferences.h"
#include "recordconfig.h"
#include "mutex.h"

#include <string.h>

#ifdef HAVE_OSS

// These are only available in commercial OSS

#ifndef AFMT_S32_LE
#define AFMT_S32_LE 	 0x00001000
#define AFMT_S32_BE 	 0x00002000
#endif


// Synchronie multiple devices by using threads

OSSThread::OSSThread(AudioOSS *device)
 : Thread(1, 0, 0)
{
	rd = 0;
	wr = 0;
	done = 0;
	this->device = device;
	input_lock = new Condition(0, "OSSThread::input_lock");
	output_lock = new Condition(1, "OSSThread::output_lock");
	read_lock = new Condition(0, "OSSThread::read_lock");
	write_lock = new Condition(0, "OSSThread::write_lock");

	// delay / timing
	bytes_written = 0;
	timer = new Timer;
	delay = 0;
	timer_lock = new Mutex("OSSThread::timer_lock");
	timer->update();
}

OSSThread::~OSSThread()
{
	done = 1;
	input_lock->unlock();
	Thread::join();
	delete input_lock;
	delete output_lock;
	delete read_lock;
	delete write_lock;
	delete timer_lock;
	delete timer;
}

void OSSThread::run()
{
	while(!done)
	{
		input_lock->lock("OSSThread::run 1");
		if(rd)
		{
			int result = read(fd, data, bytes);
			read_lock->unlock();
		}
		else
		if(wr)
		{
			if(done) return;


			Thread::enable_cancel();
			int written = write(fd, data, bytes);

// quoth the OSS documentation re. SNDCTL_DSP_GETOPTR:
// "This ioctl call should only be used when using mmap().
// In normal applications it's practically useless."
// As it does in fact seem to interact badly with at least ALSA emulation,
// go to the more complex GETODELAY route, which does seem to work.
// Long ago GETODELAY wasn't supported everywhere...

			if(written)
			{
				// Update timing
				int delay = 0;
				timer_lock->lock("OSSThread::write_buffer");
				if(!ioctl(fd, SNDCTL_DSP_GETODELAY, &delay))
					this->delay = delay;
				timer->update();
				bytes_written += written;
				timer_lock->unlock();
			}

			Thread::disable_cancel();


			if(done) return;
			write_lock->unlock();
		}
		output_lock->unlock();
	}
}

void OSSThread::write_data(int fd, unsigned char *data, int bytes)
{
	output_lock->lock("OSSThread::write_data");
	wr = 1;
	rd = 0;
	this->data = data;
	this->bytes = bytes;
	this->fd = fd;
	input_lock->unlock();
}

void OSSThread::read_data(int fd, unsigned char *data, int bytes)
{
	output_lock->lock("OSSThread::read_data");
	wr = 0;
	rd = 1;
	this->data = data;
	this->bytes = bytes;
	this->fd = fd;
	input_lock->unlock();
}

void OSSThread::wait_read()
{
	read_lock->lock("OSSThread::wait_read");
}

void OSSThread::wait_write()
{
	write_lock->lock("OSSThread::wait_write");
}


// bytes not samples
int64_t OSSThread::device_position()
{
	timer_lock->lock("OSSThread::device_position");
	int64_t ret = bytes_written - delay;
	timer_lock->unlock();
	return ret;
}










AudioOSS::AudioOSS(AudioDevice *device)
 : AudioLowLevel(device)
{
	for(int i = 0; i < MAXDEVICES; i++)
	{
		dsp_in[i] = dsp_out[i] = dsp_duplex[i] = -1;
		thread[i] = 0;
		data[i] = 0;
		data_allocated[i] = 0;
	}
}

AudioOSS::~AudioOSS()
{
}

int AudioOSS::open_input()
{
	device->in_bits = device->in_config->oss_in_bits;
// 24 bits not available in OSS
	if(device->in_bits == 24) device->in_bits = 32;

	for(int i = 0; i < MAXDEVICES; i++)
	{
		if(device->in_config->oss_enable[i])
		{
//printf("AudioOSS::open_input 10\n");
			dsp_in[i] = open(device->in_config->oss_in_device[i], O_RDONLY/* | O_NDELAY*/);
//printf("AudioOSS::open_input 20\n");
			if(dsp_in[i] < 0)
			{
				fprintf(stderr, "AudioOSS::open_input %s: %s\n", 
						device->in_config->oss_in_device[i], 
						strerror(errno));
				close_all();
				return 1;
			}
			else
			{
				int format = get_fmt(device->in_config->oss_in_bits);
				int buffer_info = sizetofrag(device->in_samples, 
					device->get_ichannels(), 
					device->in_config->oss_in_bits);

				set_cloexec_flag(dsp_in[i], 1);

// For the ice1712 the buffer must be maximum or no space will be allocated.
				if(device->driver == AUDIO_OSS_ENVY24) buffer_info = 0x7fff000f;
				if(ioctl(dsp_in[i], SNDCTL_DSP_SETFRAGMENT, &buffer_info))
				{
					printf("SNDCTL_DSP_SETFRAGMENT failed.\n");
					close_all();
					return 1;
				}
				if(ioctl(dsp_in[i], SNDCTL_DSP_SETFMT, &format) < 0)
				{
					printf("SNDCTL_DSP_SETFMT failed\n");
					close_all();
					return 1;
				}
				int channels = device->get_ichannels();
				if(ioctl(dsp_in[i], SNDCTL_DSP_CHANNELS, &channels) < 0)
				{
					printf("SNDCTL_DSP_CHANNELS failed\n");
					close_all();
					return 1;
				}
				if(ioctl(dsp_in[i], SNDCTL_DSP_SPEED, &device->in_samplerate) < 0)
				{
					printf("SNDCTL_DSP_SPEED failed\n");
					close_all();
					return 1;
				}

				audio_buf_info recinfo;
				if (ioctl(dsp_in[i], SNDCTL_DSP_GETISPACE, &recinfo))
				{
					printf("SNDCTL_DSP_GETISPACE failed\n");
					close_all();
					return 1;
				}

//printf("AudioOSS::open_input fragments=%d fragstotal=%d fragsize=%d bytes=%d\n", 
//	recinfo.fragments, recinfo.fragstotal, recinfo.fragsize, recinfo.bytes);

				thread[i] = new OSSThread(this);
				thread[i]->start();
			}
		}
	}
	return 0;
}

int AudioOSS::open_output()
{
	device->out_bits = device->out_config->oss_out_bits;
// OSS only supports 8, 16, and 32
	if(device->out_bits == 24) device->out_bits = 32;

	for(int i = 0; i < MAXDEVICES; i++)
	{
		if(device->out_config->oss_enable[i])
		{
// Linux 2.4.18 no longer supports allocating the maximum buffer size.
// Need the shrink fragment size in preferences until it works.
			dsp_out[i] = 
				open(device->out_config->oss_out_device[i], 
					O_WRONLY /*| O_NDELAY*/);
			if(dsp_out[i] < 0)
			{
				perror("AudioOSS::open_output");
				close_all();
				return 1;
			}
			else
			{
				int format = get_fmt(device->out_config->oss_out_bits);
				int buffer_info = sizetofrag(device->out_samples, 
					device->get_ochannels(), 
					device->out_config->oss_out_bits);
				audio_buf_info playinfo;

				set_cloexec_flag(dsp_out[i], 1);

// For the ice1712 the buffer must be maximum or no space will be allocated.
				if(device->driver == AUDIO_OSS_ENVY24) buffer_info = 0x7fff000f;
				if(ioctl(dsp_out[i], SNDCTL_DSP_SETFRAGMENT, &buffer_info))
				{
					printf("SNDCTL_DSP_SETFRAGMENT 2 failed.\n");
					close_all();
					return 1;
				}
				if(ioctl(dsp_out[i], SNDCTL_DSP_SETFMT, &format) < 0)
				{
					printf("SNDCTL_DSP_SETFMT 2 failed\n");
					close_all();
					return 1;
				}
				int channels = device->get_ochannels();
				if(ioctl(dsp_out[i], SNDCTL_DSP_CHANNELS, &channels) < 0)
				{
					printf("SNDCTL_DSP_CHANNELS 2 failed\n");
					close_all();
					return 1;
				}
				if(ioctl(dsp_out[i], SNDCTL_DSP_SPEED, &device->out_samplerate) < 0)
				{
					printf("SNDCTL_DSP_SPEED 2 failed\n");
					close_all();
					return 1;
				}
				if (ioctl(dsp_out[i], SNDCTL_DSP_GETOSPACE, &playinfo))
				{
					printf("SNDCTL_DSP_GETOSPACE failed\n");
					close_all();
					return 1;
				}
// printf("AudioOSS::open_output fragments=%d fragstotal=%d fragsize=%d bytes=%d\n", 
// playinfo.fragments, playinfo.fragstotal, playinfo.fragsize, playinfo.bytes);
				device->device_buffer = playinfo.bytes;
				thread[i] = new OSSThread(this);
				thread[i]->start();
			}
		}
	}
	return 0;
}

int AudioOSS::open_duplex()
{
	device->duplex_bits = device->out_config->oss_out_bits;
	if(device->duplex_bits == 24) device->duplex_bits = 32;

	for(int i = 0; i < MAXDEVICES; i++)
	{
		if(device->out_config->oss_enable[i])
		{
			dsp_duplex[i] = open(device->out_config->oss_out_device[i], O_RDWR/* | O_NDELAY*/);
			if(dsp_duplex[i] < 0)
			{
				perror("AudioOSS::open_duplex");
				close_all();
				return 1;
			}
			else
			{
				int format = get_fmt(device->out_config->oss_out_bits);
				int buffer_info = sizetofrag(device->duplex_samples, 
					device->get_ochannels(), 
					device->out_config->oss_out_bits);
				audio_buf_info playinfo;

				set_cloexec_flag(dsp_duplex[i], 1);

// For the ice1712 the buffer must be maximum or no space will be allocated.
				if(device->driver == AUDIO_OSS_ENVY24) buffer_info = 0x7fff000f;
				if(ioctl(dsp_duplex[i], SNDCTL_DSP_SETFRAGMENT, &buffer_info))
				{
					printf("SNDCTL_DSP_SETFRAGMENT failed.\n");
					close_all();
					return 1;
				}
				if(ioctl(dsp_duplex[i], SNDCTL_DSP_SETDUPLEX, 1) == -1)
				{
					printf("SNDCTL_DSP_SETDUPLEX failed\n");
					close_all();
					return 1;
				}
				if(ioctl(dsp_duplex[i], SNDCTL_DSP_SETFMT, &format) < 0)
				{
					printf("SNDCTL_DSP_SETFMT failed\n");
					close_all();
					return 1;
				}
				int channels = device->get_ochannels();
				if(ioctl(dsp_duplex[i], SNDCTL_DSP_CHANNELS, &channels) < 0)
				{
					printf("SNDCTL_DSP_CHANNELS failed\n");
					close_all();
					return 1;
				}
				if(ioctl(dsp_duplex[i], SNDCTL_DSP_SPEED, &device->duplex_samplerate) < 0)
				{
					printf("SNDCTL_DSP_SPEED failed\n");
					close_all();
					return 1;
				}
				if (ioctl(dsp_duplex[i], SNDCTL_DSP_GETOSPACE, &playinfo))
				{
					printf("SNDCTL_DSP_GETOSPACE failed\n");
					close_all();
					return 1;
				}
				device->device_buffer = playinfo.bytes;
				thread[i] = new OSSThread(this);
				thread[i]->start();
			}
		}
	}
	return 0;
}

int AudioOSS::sizetofrag(int samples, int channels, int bits)
{
	int testfrag = 2, fragsize = 1;
	samples *= channels * bits / 8
		// we want four fragments totalling this size
		/ 4;
	while(testfrag < samples)
	{
		fragsize++;
		testfrag *= 2;
	}
//printf("AudioOSS::sizetofrag %d\n", fragsize);
	return (4 << 16) | fragsize;
}

int AudioOSS::get_fmt(int bits)
{
	switch(bits)
	{
		case 32: return AFMT_S32_LE; break;
		case 16: return AFMT_S16_LE; break;
		case 8:  return AFMT_S8;  break;
	}
	return AFMT_S16_LE;
}


int AudioOSS::close_all()
{
//printf("AudioOSS::close_all 1\n");
	for(int i = 0; i < MAXDEVICES; i++)
	{
		if(dsp_in[i]) 
		{
			ioctl(dsp_in[i], SNDCTL_DSP_RESET, 0);         
			close(dsp_in[i]);      
			dsp_in[i] = -1;
		}

		if(dsp_out[i]) 
		{
//printf("AudioOSS::close_all 2\n");
			ioctl(dsp_out[i], SNDCTL_DSP_RESET, 0);        
			close(dsp_out[i]);     
			dsp_out[i] = -1;
		}

		if(dsp_duplex[i]) 
		{
			ioctl(dsp_duplex[i], SNDCTL_DSP_RESET, 0);     
			close(dsp_duplex[i]);  
			dsp_duplex[i] = -1;
		}
		
		if(thread[i]) delete thread[i];
		if(data[i]) delete [] data[i];
	}
	return 0;
}

int AudioOSS::set_cloexec_flag(int desc, int value)
{
	int oldflags = fcntl (desc, F_GETFD, 0);
	if (oldflags < 0) return oldflags;
	if(value != 0) 
		oldflags |= FD_CLOEXEC;
	else
		oldflags &= ~FD_CLOEXEC;
	return fcntl(desc, F_SETFD, oldflags);
}

int64_t AudioOSS::device_position()
{
	for (int i = 0, in_channel = 0; i < MAXDEVICES; i++)
	{
		if (thread[i])
			return thread[i]->device_position() /
				device->get_ochannels() /
				(device->get_obits()/8) +
				thread[i]->timer->get_scaled_difference(device->out_samplerate);
	}
	return 0;
}

int AudioOSS::interrupt_playback()
{
//printf("AudioOSS::interrupt_playback 1\n");
	for(int i = 0; i < MAXDEVICES; i++)
	{
		if(thread[i])
		{
			thread[i]->cancel();
			thread[i]->write_lock->unlock();
		}
	}
//printf("AudioOSS::interrupt_playback 100\n");
	return 0;
}

int AudioOSS::read_buffer(char *buffer, int bytes)
{
	int sample_size = device->get_ibits() / 8;
	int out_frame_size = device->get_ichannels() * sample_size;
	int samples = bytes / out_frame_size;

//printf("AudioOSS::read_buffer 1 %d\n", bytes);
// Fill temp buffers
	for(int i = 0; i < MAXDEVICES; i++)
	{
		if(thread[i])
		{
			int in_frame_size = device->get_ichannels() * sample_size;

			if(data[i] && data_allocated[i] < bytes)
			{
				delete [] data[i];
				data[i] = 0;
			}
			if(!data[i])
			{
				data[i] = new unsigned char[bytes];
				data_allocated[i] = bytes;
			}

			thread[i]->read_data(get_input(i), data[i], samples * in_frame_size);
		}
	}

//printf("AudioOSS::read_buffer 1 %d\n", device->get_ibits());
	for(int i = 0, out_channel = 0; i < MAXDEVICES; i++)
	{
		if(thread[i])
		{
			thread[i]->wait_read();

			for(int in_channel = 0; 
				in_channel < device->get_ichannels(); 
				in_channel++)
			{
				int in_frame_size = device->get_ichannels() * sample_size;

				for(int k = 0; k < samples; k++)
				{
					for(int l = 0; 
						l < sample_size;
						l++)
					{
						buffer[out_channel * sample_size + k * out_frame_size + l] = 
							data[i][in_channel * sample_size + k * in_frame_size + l];
					}
				}
				out_channel++;
			}
		}
	}
//printf("AudioOSS::read_buffer 2\n");
	return 0;
}

int AudioOSS::write_buffer(char *buffer, int bytes)
{
	int sample_size = device->get_obits() / 8;
	int in_frame_size = device->get_ochannels() * sample_size;
	int samples = bytes / in_frame_size;

	for(int i = 0, in_channel = 0; i < MAXDEVICES; i++)
	{
		if(thread[i])
		{
			int out_frame_size = device->get_ochannels() * sample_size;
			if(data[i] && data_allocated[i] < bytes)
			{
				delete [] data[i];
				data[i] = 0;
			}
			if(!data[i])
			{
				data[i] = new unsigned char[bytes];
				data_allocated[i] = bytes;
			}
			
			for(int out_channel = 0;
				out_channel < device->get_ochannels();
				out_channel++)
			{
				
				for(int k = 0; k < samples; k++)
				{
					for(int l = 0; l < sample_size; l++)
					{
						data[i][out_channel * sample_size + k * out_frame_size + l] = 
							buffer[in_channel * sample_size + k * in_frame_size + l];
					}
				}
				in_channel++;
			}
			
			thread[i]->write_data(get_output(i), data[i], samples * out_frame_size);
		}
	}
	for(int i = 0, in_channel = 0; i < MAXDEVICES; i++)
	{
		if(thread[i])
		{
			thread[i]->wait_write();
		}
	}
	return 0;
}

int AudioOSS::flush_device()
{
	for(int i = 0; i < MAXDEVICES; i++)
		if(thread[i]) ioctl(get_output(i), SNDCTL_DSP_SYNC, 0);
	return 0;
}

int AudioOSS::get_output(int number)
{
	if(device->w) return dsp_out[number];
	else if(device->d) return dsp_duplex[number];
	return 0;
}

int AudioOSS::get_input(int number)
{
	if(device->r) return dsp_in[number];
	else if(device->d) return dsp_duplex[number];
	return 0;
}

#endif // HAVE_OSS
