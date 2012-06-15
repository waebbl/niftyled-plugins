/*
 * libniftyled - Interface library for LED interfaces
 * Copyright (C) 2010 Daniel Hiepler <daniel@niftylight.de>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <niftyled.h>
#include "config.h"


/** some properties of our "hardware" */
struct priv
{
        LedHardware *            hw;
        char                     id[1024];
        LedCount                 ledcount;
};


/**
 * setter for plugin-specific property called "foo"
 * - this will be called if this property has been set
 * to a new value. By returning NFT_SUCCESS the new value
 * is accepted. NFT_FAILURE can be returned if value is invalid.
 */
NftResult _set_foo(LedSettings *c, NftSettingsNode *n)
{
        int baz;
        if(!nft_settings_node_prop_int_get(n, "baz", &baz))
                return NFT_FAILURE;
        
        NFT_LOG(L_DEBUG, "Foo property \"baz\" set to: %d", baz);

        char *bar;
        if(!(bar = nft_settings_node_prop_string_get(n, "bar")))
                return NFT_FAILURE;
        
        NFT_LOG(L_DEBUG, "Foo property \"bar\" set to: \"%s\"", bar);
        
        return NFT_SUCCESS;
}

/**
 * getter for plugin-specific property called "foo"
 * - this will be called if something wants to read the value
 * of this property. 
 */
static NftSettingsNode *_get_foo(LedSettings *conf, LedHardwarePlugin *plugin, void *ptr)
{
        NftSettingsNode *n;
        if(!(n = nft_settings_node_new("foo")))
                return NFT_FAILURE;

        if(!nft_settings_node_prop_string_set(n, "bar", "foobar"))
                goto _gf_error;
        
        if(!nft_settings_node_prop_int_set(n, "baz", 256))
                goto _gf_error;
        
        return n;

_gf_error:
        nft_settings_node_destroy(n);
        return NULL;
}


/**
 * called upon plugin load
 *
 * @p privdata space for a private pointer.  Set it here and it will be passed 
 * to you back later (prefer this over static variables)
 */
static NftResult _init(void **privdata, LedHardware *h)
{
        NFT_LOG(L_INFO, "Initializing plugin...");

        /* allocate private structure */
        struct priv *p;
        if(!(p = calloc(1,sizeof(struct priv))))
        {
                NFT_LOG_PERROR("calloc");
                return NFT_FAILURE;
        }
        
        /* register our config-structure */
        *privdata = p;
        
        /* save our hardware descriptor */
        p->hw = h;

        
        /** 
         * settings-property demo: register function to context that "sets"
         * a plugin-specific property
         */
	if(!(nft_settings_func_to_obj_set(led_settings_context(), led_hardware_get_propname(h, "foo"), (NftSettingsToObjFunc *) _set_foo, TRUE)))
		return NFT_FAILURE;
        /**
         * settings-property demo: register function to plugin, that "gets" 
         * a plugin-specific property 
         */
	if(!(nft_settings_func_from_obj_set(led_settings_context(), led_hardware_get_plugin(h), (NftSettingsFromObjFunc *) _get_foo)))
		return NFT_FAILURE;


        
        return NFT_SUCCESS;
}


/**
 * called upon plugin unload
 */
static void _deinit(void *privdata)
{
        NFT_LOG(L_INFO, "Deinitializing plugin...");   

        struct priv *p = privdata;
        
        /** unregister or settings-handlers */
        nft_settings_func_to_obj_unset(led_settings_context(), led_hardware_get_propname(p->hw, "foo"));
        nft_settings_func_from_obj_unset(led_settings_context(), led_hardware_get_plugin(p->hw));
}



/**
 * initialize hardware
 */
static NftResult _hw_init(void *privdata, const char *id)
{
        NFT_LOG(L_DEBUG, "Initializing dummy hardware");

        struct priv *p = privdata;

        /* ... do check ... */
        
        /* pixelformat supported? */
        const char *fmtstring = led_pixel_format_to_string(led_chain_get_format(
                                                led_hardware_get_chain(p->hw)));
        NFT_LOG(L_DEBUG, "Using \"%s\" as pixel-format", fmtstring);
        
        /* dummy should support any format but in theory, you could 
           check like this ... */
        //if(strcmp(fmtstring, "RGB u8") != 0)
        //        return NFT_FAILURE;
        
        /* ...or this ... */
        //LedPixelFormat *format = led_chain_get_format(chain);
        //if(led_frame_format_get_bytes_per_pixel(format) ...)
        //if(led_frame_format_get_n_components(format) ...)
        // ...

                
        /* copy our id (and/or change it; check for "*" wildcard) */
        strncpy(p->id, id, sizeof(p->id));
        
        return NFT_SUCCESS;
}


/**
 * deinitialize hardware
 */
static void _hw_deinit(void *privdata)
{
        NFT_LOG(L_DEBUG, "Deinitializing dummy hardware");
}


/**
 * plugin getter - this will be called if core wants to get stuff from the plugin
 * @note you don't need to implement a getter for every LedPluginObj
 */
NftResult _get_handler(void *privdata, LedPluginObj o, LedPluginObjData *data)
{
        struct priv *p = privdata;
        
        /** decide about object to give back to the core (s. hardware.h) */
        switch(o)
        {
                case LED_HW_ID:
                {
                        data->id = p->id;
                        return NFT_SUCCESS;
                }

                case LED_HW_LEDCOUNT:
                {
                        data->ledcount = p->ledcount;
                        return NFT_SUCCESS;
                }
                        
                default:
                {
                        NFT_LOG(L_ERROR, "Request to get unhandled object \"%s\" from plugin",
                                        led_hardware_get_plugin_obj_name(o));
                        return NFT_FAILURE;
                }
        }

        return NFT_FAILURE;
}


/**
 * plugin setter - this will be called if core wants to set stuff
 * @note you don't need to implement a setter for every LedPluginObj
 */
NftResult _set_handler(void *privdata, LedPluginObj o, LedPluginObjData *data)
{
        struct priv *p = privdata;
        
        /** decide about type of data (s. hardware.h) */
        switch(o)
        {
                case LED_HW_ID:
                {
                        strncpy(p->id, data->id, sizeof(p->id));
                        return NFT_SUCCESS;
                }

                case LED_HW_LEDCOUNT:
                {
                        p->ledcount = data->ledcount;
                        return NFT_SUCCESS;
                }

                default:
                {
                        return NFT_SUCCESS;
                }
        }
        
        return NFT_FAILURE;
}


/**
 * trigger hardware to show data
 *
 * - if you previously uploaded your data to your hardware, output it now to LEDs
 * - if you can't do this, try to send all data except last bit/value/... in 
 *   _send(). Then send last portion here
 * - if you can't do this, send all data here as quick as possible :)
 */
NftResult _show(void *privdata)
{
        NFT_LOG(L_DEBUG, "Showing dummy data");
        return NFT_SUCCESS;
}


/**
 * send data in chain to hardware (only use this if hardware doesn't show data right away
 * to avoid blanking)
 */
NftResult _send(void *privdata, LedChain *c, LedCount count, LedCount offset)
{
        NFT_LOG(L_VERBOSE, "Sending dummy data");       

        /* do nothing if no greyscale-values are not printed */
        if(nft_log_level_get() > L_DEBUG)
                return NFT_SUCCESS;

        /* print all greyscale-values */
        NFT_LOG(L_DEBUG, "Greyscale-Buffer:");

        char *buffer = led_chain_get_buffer(c);
        int i,a ;
        size_t pos;
        static char string[512];
        char *b = buffer;
        
        for(a=offset; a<count/10; a++)
        {
                char *tmp = string;
                size_t size = sizeof(string);
                
                for(i=0; i<10; i++)
                {
                        //if(tmp >= string+count)
                                //return NFT_SUCCESS;

                        if((pos = snprintf(tmp, size, "0x%.2hhx ",*b++)) < 0)
                        {
                                NFT_LOG_PERROR("snprintf()");
                                return NFT_FAILURE;
                        }

                        tmp += pos;
                        size -= pos;
                }

                NFT_LOG(L_DEBUG, "%s", string);
        }

        /* send chainbuffer to hardware */
        NFT_LOG(L_DEBUG, "Sent %d LED values to dummy hardware", led_chain_get_ledcount(c));
        
        return NFT_SUCCESS;
}


/** descriptor of hardware-plugin passed to the library */
LedHardwarePlugin hardware_descriptor =
{
        /** family name of the plugin (lib{family}-hardware.so) */
        .family = "dummy",
        /** api version */
        .api_version = LED_HARDWARE_API,
        /** plugin version major */
        .major_version = 0,
        /** plugin version minor */
        .minor_version = 0,
        /** plugin version micro */
        .micro_version = 1,
        .license = "GPL",
        .author = "Daniel Hiepler <daniel@niftylight.de> (c) 2011",
        .description = "Dummy hardware plugin",
        .url = PACKAGE_URL,
        .id_example = "any printable string",
        .plugin_init = _init,
        .plugin_deinit = _deinit,
        .hw_init = _hw_init,
        .hw_deinit = _hw_deinit,
        .get = _get_handler,
        .set = _set_handler,
        .show = _show,
        .send = _send,
};
