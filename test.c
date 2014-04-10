#include <stdio.h>
    #include <stdlib.h>
    #include <libconfig.h>
    
    int main(int argc, char **argv)
    {
         
        const config_setting_t *device,*zoneName;
        const char *base = NULL;
        int count, n, enabled,usercode;

        config_t cfg, *cf;
        cf = &cfg; 
        
        config_init(cf);
    
        if (!config_read_file(cf, "/etc/pmaxd.conf")) {
            fprintf(stderr, "%s:%d - %s\n",
                config_error_file(cf),
                config_error_line(cf),
                config_error_text(cf));
            config_destroy(cf);
            return(EXIT_FAILURE);
        }
      
      
      
        if (config_lookup_int(cf, "usercode", &usercode))
          printf("usercode: %04X\n", usercode);          

    
        device = config_lookup(cf, "device");
        count = config_setting_length(device);
    
        printf("I have %d retries:\n", count);
        for (n = 0; n < count; n++) {
          printf( "\t#%d. %s\n" , n + 1 , config_setting_get_string_elem(device, n) );
        }
        
        zoneName = config_lookup(cf, "zonename");
        count = config_setting_length(zoneName);
    
        printf("I have %d zone:\n", count);
        for (n = 0; n < count; n++) {
            printf("\t#%d. %s\n", n + 1,
                config_setting_get_string_elem(zoneName, n));
        }
        
    
        config_destroy(cf);
        return 0;
    }