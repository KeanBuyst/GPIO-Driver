#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include<linux/proc_fs.h>
#include<linux/slab.h>

#include<asm/io.h>

#define MAX_USER_SIZE 1024
#define BCM2711_GPIO_ADDRESS 0xfe200000

typedef unsigned int uint;

static struct proc_dir_entry *gp_proc = NULL;

static char data_buffer[MAX_USER_SIZE+1] = {0};
static char response_buffer[1] = {0};
static uint *gpio_registers = NULL;

static uint HIGH_BUFFER = 0x0;
static uint LOW_BUFFER = 0x0;

static void print(char* text)
{
  printk("GPIO: %s\n",text);
}

static uint* getGPFSEL(uint pin)
{
  uint fsel_index = pin/10;
  return gpio_registers + fsel_index;
}

static uint getGPFSEL_BP(uint pin)
{
  return pin % 10;
}

ssize_t read(struct file *file, char __user *user,size_t size, loff_t *off)
{
  if (copy_to_user(user,response_buffer,1))
  {
    print("failed to copy into user buffer");
    return size;
  }
  return 1;
}

ssize_t write(struct file *file, const char __user *user,size_t size, loff_t *off)
{
  char instruction;
  uint pin;
  uint value;

  memset(data_buffer,0x0,sizeof(data_buffer));

  if (size > MAX_USER_SIZE)
  {
    size = MAX_USER_SIZE;
  }

  if(copy_from_user(data_buffer,user,size))
  {
    print("failed to copy from user buffer");
    return size;
  }

  if (sscanf(data_buffer,"%c %u %u",&instruction,&pin,&value) != 3)
  {
    print("invalid data format recived");
    return size;
  }

  if (pin > 27 || pin < 0)
  {
    print("invalid pin accessed, must be between 27 & 0");
    return size;
  }

  if (value != 1 && value != 0)
  {
    print("invalid binary ( not 1 or 0 )");
    return size;
  }

  switch (instruction)
  {
    case 'g':
      {
        uint bitpos = getGPFSEL_BP(pin);
        uint* gpio_fsel = getGPFSEL(pin);

        // clear register + set gpio to input
        *gpio_fsel &= ~(7 << (bitpos * 3));
        // set to output
        if (value == 1)
        {
          *gpio_fsel |= (1 << (bitpos * 3));
        }
      }
      break;
    case 'o':
      {
        if (value == 1)
        {
          // turn pin high
          uint* reg = (uint*)((char*)gpio_registers + 0x1c);
          HIGH_BUFFER |= (1 << pin);
          LOW_BUFFER &= ~(1 << pin);
          *reg = HIGH_BUFFER;
        }
        else 
        {
          // turn pin to low
          uint* reg = (uint*)((char*)gpio_registers + 0x28);
          LOW_BUFFER |= (1 << pin);
          HIGH_BUFFER &= ~(1 << pin);
          *reg = LOW_BUFFER;
        }
      }
      break;
    case 'l':
      {
        uint* reg = (uint*)((char*)gpio_registers + 0x34);
        uint bit = (*reg >> pin) & 1;
        sprintf(response_buffer,"%u",bit);
      }
      break;
    default:
      print("invalid instruction passed");
      return size;
  }

  return size;
}
static const struct proc_ops gp_proc_fs = 
{
  .proc_read = read,
  .proc_write = write,
};

static int __init driver_init(void)
{
  print("warming pins...");

  gpio_registers = (int*)ioremap(BCM2711_GPIO_ADDRESS, PAGE_SIZE);
  if (gpio_registers == NULL)
  {
    print("failed to map GPIO memory");
    return -1;
  }

  gp_proc = proc_create("gpio",0666,NULL,&gp_proc_fs);
  if (gp_proc == NULL)
  {
    print("failed to create proc");
    return -1;
  }

  print("fully initialized gpio");
  return 0;
}

static void __exit driver_exit(void)
{
  print("driver closing");
  iounmap(gpio_registers);
  proc_remove(gp_proc);
  return;
}

module_init(driver_init);
module_exit(driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kean Buyst");
MODULE_DESCRIPTION("GPIO Interface");
MODULE_VERSION("1.0");
