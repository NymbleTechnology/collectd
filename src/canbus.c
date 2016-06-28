/**
    canbus.c
*/

/*
  Note:
    - plugins are executed in parallel, thus, thread-safe functions must to be used
    - module_register is the only required function below. It is responsible
      for registering all the other entry points that collectd should call.
 */

#if ! HAVE_CONFIG_H

#include <stdlib.h>
#include <string.h>

#ifndef __USE_ISOC99 /* required for NAN */
# define DISABLE_ISOC99 1
# define __USE_ISOC99 1
#endif /* !defined(__USE_ISOC99) */
#include <math.h>
#if DISABLE_ISOC99
# undef DISABLE_ISOC99
# undef __USE_ISOC99
#endif /* DISABLE_ISOC99 */

#include <time.h>

#endif /* ! HAVE_CONFIG */

#include <float.h>

#include <collectd.h>
#include <common.h>
#include <plugin.h>

#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>


#define HOSTNAME  "localhost"
#define PLUGIN    "canbus"

/**
 data_source_t - data source definition:
  - name of the data source
  - type of the data source
    (either DS_TYPE_COUNTER, DS_TYPE_GAUGE,
     DS_TYPE_DERIVE or DS_TYPE_ABSOLUTE)
  - minimum allowed value
  - maximum allowed value

 data_set_t - data set definition:
  - name of the data set
  - number of data sources
  - list of data sources

NOTE: If you're defining a custom data-set, you have to make that
      known to any servers as well. Else, the server is not able
      to store values using the type defined by that data-set.
      It is strongly recommended to use one of the types and
      data-sets pre-defined in the types.db file.
*/

static data_source_t dsSimpleGauge[1] =
{{ "value", DS_TYPE_GAUGE, NAN, NAN }};

typedef struct { gauge_t value; } tSimpleGauge;

static data_source_t dsSimpleCounter[1] =
{{ "value", DS_TYPE_COUNTER, NAN, NAN }};

typedef struct { counter_t value; } tSimpleCounter;

static data_source_t dsBoundsGauge[4] =
{
    { "last",    DS_TYPE_GAUGE, NAN, NAN },
    { "average", DS_TYPE_GAUGE, NAN, NAN },
    { "minimum", DS_TYPE_GAUGE, NAN, NAN },
    { "maximum", DS_TYPE_GAUGE, NAN, NAN }
};

typedef struct
{
    /* MUST be in this order */
    gauge_t   last, average, minimum, maximum;
    /* MUST be in this order */

    double    sum;
    int       count;

} tBoundsGauge;

typedef unsigned int tSequenceID;
typedef enum { kSimpleGauge, kSimpleCounter, kBoundsGauge } tGaugeType;

typedef struct
{
    tGaugeType  type;
    union {
        tSimpleGauge    *gauge;
        tSimpleCounter  *counter;
        tBoundsGauge    *bounds;
    } data;

    data_set_t    dataSet;
    value_list_t  valueList;

    cdtime_t      lastChange;
    tSequenceID   seqID;

} tDataSet;

/* global state */
static struct
{
    unsigned int    seqID;

    /* * * CAN message 0x210 * * */
    tSimpleGauge  powerPackID;  /* should never change... */
    struct
    {
        tSimpleCounter  power;  /* only increases */
        tSimpleCounter  hours;  /* only increases */

    } accumulated;
    /* mechanical switch that indicates the refueling port is connected */
    tSimpleGauge  refuelPort; /* don't bother tracking min/max seen */

    /* * * CAN message 0x211 * * */
    struct
    {
        tBoundsGauge  voltage;
        tBoundsGauge  current;

    } input, output;

    /* * * CAN message 0x212 * * */
    struct
    {
        tBoundsGauge  pressure;     /* units of 0.1 bar */
        tBoundsGauge  temperature;  /* units of 0.1 degrees centigrade */

    } h2tank;                       /* hydrogen fuel tank */
    tSimpleGauge   errorCode;       /* IEEE double has a 53 bit mantissa, so a 32 bit int will fit just fine */

} fuelCell;

typedef enum
{
	kPowerPackID,
	kAccumulatedPower,
	kAccumulatedHours,
	kRefuelPort,
	kInputVoltage,
	kInputCurrent,
	kOutputVoltage,
	kOutputCurrent,
	kH2tankPressure,
	kH2tankTemperature,
	kErrorCode,
	kNumDataSet

} eDataSetIdx;

/*
    Core data structures, including the data_set_t and value_list_t data structures,
    which the plugin API is based upon.
    Statically initialized wherever possible, to cut down runtime intialization.
*/
tDataSet dataSet[kNumDataSet] =
{
    /* * * CAN message 0x210 * * */
    [kPowerPackID] = {
        kSimpleGauge, { .gauge = &fuelCell.powerPackID },
        { "powerPackID", 1, dsSimpleGauge },
        { (value_t *)&fuelCell.powerPackID.value, 1, 0, 0, HOSTNAME, PLUGIN, "", "powerPackID", "", NULL }
    },
    [kAccumulatedPower] = {
        kSimpleCounter, { .counter = &fuelCell.accumulated.power },
        { "accumulatedPower", 1, dsSimpleCounter },
        { (value_t *)&fuelCell.accumulated.power.value, 1, 0, 0, HOSTNAME, PLUGIN, "", "accumulatedPower", "", NULL }
    },
    [kAccumulatedHours] = {
        kSimpleCounter, { .counter = &fuelCell.accumulated.hours },
        { "accumulatedHours", 1, dsSimpleCounter },
        { (value_t *)&fuelCell.accumulated.hours.value, 1, 0, 0, HOSTNAME, PLUGIN, "", "accumulatedHours", "", NULL }
    },
    [kRefuelPort] = {
        kSimpleGauge, { .gauge = &fuelCell.refuelPort },
        { "refuelPort", 1, dsSimpleGauge },
        { (value_t *)&fuelCell.refuelPort.value, 1, 0, 0, HOSTNAME, PLUGIN, "", "refuelPort", "", NULL }
    },

    /* * * CAN message 0x211 * * */
    [kInputVoltage] = {
        kBoundsGauge, { .bounds = &fuelCell.input.voltage },
        { "inputVoltage", 4, dsBoundsGauge },
        { (value_t *)&fuelCell.input.voltage.last, 4, 0, 0, HOSTNAME, PLUGIN, "", "inputVoltage", "", NULL }
    },
    [kInputCurrent] = {
        kBoundsGauge, { .bounds = &fuelCell.input.current },
        { "inputCurrent", 4, dsBoundsGauge },
        { (value_t *)&fuelCell.input.current.last, 4, 0, 0, HOSTNAME, PLUGIN, "", "inputCurrent", "", NULL }
    },
    [kOutputVoltage] = {
        kBoundsGauge, { .bounds = &fuelCell.output.voltage },
        { "outputVoltage", 4, dsBoundsGauge },
        { (value_t *)&fuelCell.output.voltage.last, 4, 0, 0, HOSTNAME, PLUGIN, "", "outputVoltage", "", NULL }
    },
    [kOutputCurrent] = {
        kBoundsGauge, { .bounds = &fuelCell.output.current },
        { "outputCurrent", 4, dsBoundsGauge },
        { (value_t *)&fuelCell.output.current.last, 4, 0, 0, HOSTNAME, PLUGIN, "", "outputCurrent", "", NULL }
    },

    /* * * CAN message 0x212 * * */
    [kH2tankPressure] = {
        kBoundsGauge, { .bounds = &fuelCell.h2tank.pressure },
        { "tankPressure", 4, dsBoundsGauge },
        { (value_t *)&fuelCell.h2tank.pressure.last, 4, 0, 0, HOSTNAME, PLUGIN, "", "tankPressure", "", NULL }
    },
    [kH2tankTemperature] = {
        kBoundsGauge, { .bounds = &fuelCell.h2tank.temperature },
        { "tankTemperature", 4, dsBoundsGauge   },
        { (value_t *)&fuelCell.h2tank.temperature.last, 4, 0, 0, HOSTNAME, PLUGIN, "", "tankTemperature", "", NULL }
    },
    [kErrorCode] = {
        kSimpleGauge, { .gauge = &fuelCell.errorCode },
        { "errorCode", 1, dsSimpleGauge   },
        { (value_t *)&fuelCell.errorCode.value, 1, 0, 0, HOSTNAME, PLUGIN, "", "errorCode", "", NULL }
    }
};

#define NUM_CANBUS 1
static struct
{
    int socket;

} CANbus[ NUM_CANBUS ];


void updateSimpleGauge( eDataSetIdx ds, gauge_t newValue )
{
    dataSet[ds].lastChange = cdtime();
    dataSet[ds].seqID = fuelCell.seqID; // mark it dirty
    dataSet[ds].data.gauge->value = newValue;
}

void updateSimpleCounter( eDataSetIdx ds, counter_t newValue )
{
    dataSet[ds].lastChange = cdtime();
    dataSet[ds].seqID = fuelCell.seqID; // mark it dirty
    dataSet[ds].data.counter->value = newValue;
}

void updateBoundsGauge( eDataSetIdx ds, gauge_t newValue )
{
    tBoundsGauge *bounds = dataSet[ds].data.bounds;

    dataSet[ds].lastChange = cdtime();
    dataSet[ds].seqID = fuelCell.seqID; // mark it 'dirty'

    bounds->sum += newValue;
    ++bounds->count;
    bounds->average = bounds->sum / bounds->count;

    if ( bounds->last != newValue )
    {
        bounds->last = newValue;

        if ( newValue < bounds->minimum )
        {
            bounds->minimum = newValue;
        }
        if ( newValue > bounds->maximum )
        {
            bounds->maximum = newValue;
        }
    }
}

void resetBoundsGauge( eDataSetIdx ds )
{
    tBoundsGauge *bounds = dataSet[ds].data.bounds;

    if (bounds->count > 0)
    {
        bounds->sum  /= bounds->count;
        bounds->count = 1;
    }
    bounds->minimum = bounds->last;
    bounds->maximum = bounds->last;
}

static int CreateCANsocket( int interface )
{
    int                  result;
    struct ifreq         ifr;
    struct sockaddr_can  addr;

    result = -3;
    if ( interface >= NUM_CANBUS )
    {
        plugin_log(LOG_ERR, "CAN interface %d does not exist", interface);
    }
    else
    {
        result = -2;
        /* Create the socket */
        CANbus[interface].socket = socket( PF_CAN, SOCK_RAW, CAN_RAW );

        if ( CANbus[interface].socket == -1 )
        {
            plugin_log(LOG_ERR, "canbus: could not create a CAN socket (%d - %s)", errno, strerror(errno));
        }
        else
        {
            /* Locate the interface you wish to use */
            sprintf( ifr.ifr_name, "can%d", interface );
            result = ioctl( CANbus[interface].socket, SIOCGIFINDEX, &ifr );
            /* if successful, Ifr.ifr_ifindex will be set to that device's index */
            if ( result == -1 )
            {
                plugin_log(LOG_ERR, "canbus: could not find %s (%d - %s)", ifr.ifr_name, errno, strerror(errno));
            }
            else
            {
                /* Select that CAN interface, and bind the socket to it.*/
                addr.can_family  = AF_CAN;
                addr.can_ifindex = ifr.ifr_ifindex;
                result = bind( CANbus[interface].socket, (struct sockaddr *)&addr, sizeof( addr ) );
                if (result == -1)
                {
                    plugin_log(LOG_ERR, "canbus: unable to bind to %s (%d - %s)", ifr.ifr_name, errno, strerror(errno));
                }
                else
                {
                    plugin_log(LOG_INFO, "canbus: bound to %s", ifr.ifr_name);
                }
            }
        }
    }
    return result;
}

static int DestroyCANsocket( int interface )
{
    if ( CANbus[interface].socket > STDERR_FILENO )
    {
        close( CANbus[interface].socket );
        CANbus[interface].socket = -1;
    }
    return 0;
}

void SendTimeMsg( void )
{
    struct timeval  now;
    struct tm       *time;
    unsigned char   body[8];

    if ( gettimeofday( &now, NULL ) == 0 )
    {
        time = gmtime( &now.tv_sec );

        body[0] = time->tm_year - 100;  /* year  (0-99) */
        body[1] = time->tm_mon + 1;     /* month (1-12) */
        body[2] = time->tm_mday;        /* day   (1-31) */

        body[3] = time->tm_hour;        /* hours      (0-23) */
        body[4] = time->tm_min;         /* minutes    (0-59) */
        body[5] = time->tm_sec;         /* seconds    (0-59) */
        body[6] = now.tv_usec / 10000;  /* hundredths (0-99) */

        body[7] = 0;                    /* 'fuel level' - not currently used */
    }
    body[0] = body[0];
}

#undef  SIMULATED
#ifdef  SIMULATED
int simulateCANmessage( struct can_frame *frame, size_t size)
{
static canid_t canID = 0x210;

    int len = 8;
    unsigned char *body = frame->data;

    frame->can_id = canID;
    switch ( canID & CAN_SFF_MASK )
    {
    case 0x210:
        usleep( 100000 );

        /* Powerpack ID */
        body[0] = 0x34;
        body[1] = 0x12;
        /* accumulated power */
        body[2] = random();
        body[3] = random();
        /* accumulated hours */
        body[4] = random();
        body[5] = random();
        body[6] = random();
        /* refuel port detect */
        body[7] = random() & 1;

        canID = 0x211;  /* next frame to send */
        break;

    case 0x211:
        /* output current */
        body[0] = random();
        body[1] = random();
        /* input current */
        body[2] = random();
        body[3] = random();
        /* output voltage */
        body[4] = random();
        body[5] = (480 >> 8);
        /* input voltage */
        body[6] = random();
        body[7] = (520 >> 8);

        canID = 0x212;  /* next frame to send */
        break;

    case 0x212:
        /* H2 Tank Pressure */
        body[0] = random();
        body[1] = random();
        /* H2 Tank Temperature */
        body[2] = random();
        body[3] = random();
        /* Error Code */
        body[4] = 0x78;
        body[5] = 0x56;
        body[6] = 0x34;
        body[7] = 0x12;

        canID = 0x210;  /* next frame to send */
        break;

    default:
        canID = 0x210 + CAN_ERR_FLAG;  /* should never happen, but... */
        len = 0;
        break;
    }

    return len;
}
#endif

static struct
{
    pthread_t       thread;
    pthread_attr_t  threadAttr;
    pthread_mutex_t lock;

} background;


/**
    This thread receives CAN packets from the CAN socket,
    parses the packet and updates the corresponding variables
*/

void *backgroundThread( void *arg )
{
    struct can_frame frame;
    unsigned char   *body;
    unsigned long    canID;
    signed int       len;

    do {
#ifndef SIMULATED
        if ( CANbus[0].socket < 1 )
        {
            sleep(1);   /* wait for CAN socket to become valid */
        }
        else
        {
            len = read( CANbus[0].socket, &frame, sizeof(frame) );
#else
        {
            len = simulateCANmessage( &frame, sizeof(frame) );
#endif // SIMULATED

            if ( len < 8 )
            {
                /* log runt frame */
            } else {
                if ( (frame.can_id & CAN_ERR_FLAG) != 0 )
                {
                    /* log error frame */
                }
                else
                {
                    canID = frame.can_id & (frame.can_id & CAN_EFF_FLAG ? CAN_EFF_MASK : CAN_SFF_MASK);
                    body  = frame.data;

                    pthread_mutex_lock( &background.lock );

                    switch ( canID )
                    {
                    case 0x210:
                        updateSimpleGauge(   kPowerPackID,      (body[1] << 8) | body[0] );
                        updateSimpleCounter( kAccumulatedPower, (body[3] << 8) | body[2] );
                        updateSimpleCounter( kAccumulatedHours, (body[6] << 16) | (body[5] << 8) | body[4] );
                        updateSimpleGauge(   kRefuelPort,       body[7] );
                        break;

                    case 0x211:
                        updateBoundsGauge( kOutputCurrent, ((body[1] << 8) | body[0]) / 10 );
                        updateBoundsGauge( kInputCurrent,  ((body[3] << 8) | body[2]) / 10 );
                        updateBoundsGauge( kOutputVoltage, ((body[5] << 8) | body[4]) / 10 );
                        updateBoundsGauge( kInputVoltage,  ((body[7] << 8) | body[6]) / 10 );
                        break;

                    case 0x212:
                        updateBoundsGauge( kH2tankPressure,    ((body[1] << 8) | body[0]) / 10 );
                        updateBoundsGauge( kH2tankTemperature, ((body[3] << 8) | body[2]) / 10 );
                        updateSimpleGauge( kErrorCode,
                                           (body[7] << 24) | (body[6] << 16) | (body[5] << 8) | body[4] );
                        break;

                    default:
                        break;
                    }

                    pthread_mutex_unlock( &background.lock );
                }
            }
        }
    } while (1);

    return arg;
}

/**
    Called once upon startup to initialize the plugin.

    Return value != 0 indicates an error and causes
    the plugin to be disabled.
*/
static int CANbusInit( void )
{
    int result;
    int i;
    /* Set up anything that can't easily be statically initialized */

    for ( i = 0; i < kNumDataSet; ++i )
    {
        /* fix up the host field of all value_list_t structures */
        sstrncpy( dataSet[i].valueList.host, hostname_g, DATA_MAX_NAME_LEN );

        if (dataSet[i].type == kBoundsGauge)
            { resetBoundsGauge( i ); }
    }

    CreateCANsocket( 0 );

    /* open sockets, initialize data structures, ... */
    result = pthread_mutex_init( &background.lock, NULL );
    if ( result == 0 )
    {
        result = plugin_thread_create( &background.thread, &background.threadAttr, backgroundThread, NULL );
    }
    return result;
}

/**
    Called before shutting down collectd.
*/
static int CANbusShutdown( void )
{
    /* close sockets, free data structures, ... */

    DestroyCANsocket( 0 );

    return 0;
}

/**
    Called at regular intervals to capture the current values.
*/
static int CANbusRead( void )
{
    int i;

    /* grab the lock, to serialize access to the CANbus parameters  */
    pthread_mutex_lock( &background.lock );

    /* dispatch any values that have changed since the last read */
    for ( i = 0; i < kNumDataSet; ++i )
    {
        if ( dataSet[i].seqID == fuelCell.seqID )
        {
            /* dispatch the values to collectd core, which passes
               them on to all configured write functions */
            plugin_dispatch_values( &dataSet[i].valueList );

            if (dataSet[i].type == kBoundsGauge)
                { resetBoundsGauge( i ); }
        }
    }

    /* increasing the sequence ID makes all variables 'clean' again */
    ++fuelCell.seqID;

    pthread_mutex_unlock( &background.lock );

    /* A return value != 0 indicates an error and the plugin
       will be skipped for an increasing amount of time. */
    return 0;
}


/**
    Called when plugin_dispatch_notification () has been used.
*/
static int CANbusNotify( const notification_t *notif, user_data_t *ud )
{
    int  result;
    char timeStr[RFC3339_SIZE];
    char nameStr[512];

    if ( rfc3339( timeStr, sizeof(timeStr), notif->time ) != 0 )
    {
        timeStr[0] = '\0';
    }

    printf( "NOTIF (%s): %i - ", timeStr, notif->severity );

    result = format_name( nameStr, sizeof( nameStr ), notif->host,
                          notif->plugin, notif->plugin_instance,
                          notif->type,   notif->type_instance );

    if ( result == 0 )
        { printf( "%s ", nameStr ); }

    printf( "%s\n", notif->message );

    return result;
}


/**
    Called immediately after loading the plugin. Must
    register our entry points with the collectd core.
*/
void module_register( void )
{
    int i;

    plugin_register_init( PLUGIN, CANbusInit );

    /* register all of our data sets */
    for ( i = 0; i < kNumDataSet; ++i )
        { plugin_register_data_set( &dataSet[i].dataSet ); }

    plugin_register_read( PLUGIN, CANbusRead );
    plugin_register_notification( PLUGIN, CANbusNotify, NULL );

    plugin_register_shutdown( PLUGIN, CANbusShutdown );
}
