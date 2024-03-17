/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#include <Arduino.h>

#include "include/lcosb_echo.h"

#define DL_SHIFT 6 // (double)(*0.343/2) = 0.1715 ~ 0.171875 = 11/64 = (long)*11 >> 6
#define DL2MM_CALC(dl) ((int)( (dl * 11) >> DL_SHIFT ))

typedef unsigned long ulong_t;

echo_rec_buff_t* ECHO_ECHO_BUFF = NULL; 
ulong_t last_read_tp;
ulong_t last_write_tp;

static pl_rec_buff_t ECHO_PL_STORE = {
    .BUFF   = NULL,
    ._size  = 0,
    ._dropped   = 0,
    ._leak      = 0,
};

void SetupEcho() {
  pinMode(ECHO_TRIGGER, OUTPUT);
  pinMode(ECHO_PIN_0, INPUT);
  pinMode(ECHO_PIN_1, INPUT);
}










/** Buffer Maintainance
 * 
*/

// debug-testting
int initEchoBuff() {
    {   // Debug logs
        Serial.println("Initializing Echo Buffer...");
    }

    echo_rec_buff_t* eb = (echo_rec_buff_t*)malloc(sizeof(echo_rec_buff_t));
    if (!eb) {
        #if LCOSB_DEBUG_LVL > LCOSB_ERR
		{   // Debug logs
            Serial.println("Memory allocation for echo_rec_buff_t failed. Exiting.");
        }
		#endif
        return 1;
    }

    // Initializing with a single element
    eb->NR = echo_create_rec(millis());
    eb->NR->next = eb->NR; // loop over

    eb->LW = eb->NR;
    eb->_dropped = 0, eb->_leak = 0, eb->_size = 1;

    #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
	// Debug logs
	Serial.print("Created new echo_rec_buff_t at address: ");
	Serial.println(reinterpret_cast<uintptr_t>(eb), 16);
	Serial.print("NR set to new echo_record_t at address: ");
	Serial.println(reinterpret_cast<uintptr_t>(eb->NR), 16);
	Serial.print("LW set to NR at address: ");
	Serial.println(reinterpret_cast<uintptr_t>(eb->LW), 16);
	Serial.print("_dropped set to ");
	Serial.println(eb->_dropped);
	Serial.print("_leak set to ");
	Serial.println(eb->_leak);
	Serial.print("_size set to ");
	Serial.println(eb->_size);
	# endif

    last_read_tp = millis();

	ECHO_ECHO_BUFF = eb;

	#if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
    {   // Debug logs
        Serial.println("Echo Buffer initialized successfully [pointer-copy-global].");    
	}
	#endif

    return 0;
}

int initPLBuff(int buff_size) {
    ECHO_PL_STORE.BUFF = (lcosb_echo_pl_t**) malloc(sizeof(lcosb_echo_pl_t*)*buff_size);

    if (!ECHO_PL_STORE.BUFF) {
        #if LCOSB_DEBUG_LVL > LCOSB_ERR
		{   // Debug logs
            Serial.println("Memory allocation for ECHO_PL_STORE.BUFF failed. Exiting.");
        }
		#endif
        return 1;
    }

    ECHO_PL_STORE.LW    = 0;
    ECHO_PL_STORE.NR    = 0;
    ECHO_PL_STORE._size = buff_size;

    ECHO_PL_STORE._dropped  = 0;
    ECHO_PL_STORE._leak     = 0;

    // no initial data stored in PL_BUFF
    #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
    {   // Debug logs
        Serial.print("PL Buffer/Store initialized successfully with _size: ");
        Serial.println(ECHO_PL_STORE._size);
	}
	#endif

    return 0;
}

int echo_writePLStore(lcosb_echo_pl_t* new_pl) {
    if (new_pl == NULL) {
		#if LCOSB_DEBUG_LVL > LCOSB_ERR
        Serial.println("Received NULL pointer. Exiting[2]");
        #endif
        return 2;
    } 
	#if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
	else { 
		// debug log
        Serial.print("Good Pointer Recieved, new_pl (");
        Serial.print(reinterpret_cast<uintptr_t>(new_pl), 16);
        Serial.println(")");
    }
	#endif

    // chk BUFF init
    if (ECHO_PL_STORE.BUFF == NULL) {
        #if LCOSB_DEBUG_LVL > LCOSB_ERR
        	Serial.println("ECHO_PL_STORE.BUFF is NULL pointer. call initPLBuff(). Exiting[2]");
        #endif
        return 2;
    }

    // Check if the buffer is full
    if ((ECHO_PL_STORE.LW + 1) % ECHO_PL_STORE._size == ECHO_PL_STORE.NR) {
        // Buffer is full, data will be dropped
        ECHO_PL_STORE._dropped++;
        return 1;
    }

    // Write data to the buffer
    ECHO_PL_STORE.BUFF[ECHO_PL_STORE.LW+1] = new_pl;
    ECHO_PL_STORE.LW = (ECHO_PL_STORE.LW + 1) % ECHO_PL_STORE._size;

    return 0;  // Write successful
}

int echo_readPLStore(lcosb_echo_pl_t** data) {
    // chk BUFF init
    if (ECHO_PL_STORE.BUFF == NULL) {
        #if LCOSB_DEBUG_LVL > LCOSB_ERR
        	Serial.println("ECHO_PL_STORE.BUFF is NULL pointer. call initPLBuff(). Exiting[2]");
        #endif
        return 2;
    }

    // Check if the buffer is empty
    if (ECHO_PL_STORE.NR == ECHO_PL_STORE.LW) {
        // Buffer is empty
        return 1;
    }

    // Read data from the buffer
    *data = ECHO_PL_STORE.BUFF[ECHO_PL_STORE.NR];
    ECHO_PL_STORE.NR = (ECHO_PL_STORE.NR + 1) % ECHO_PL_STORE._size;

    return 0;  // Read successful
}


echo_record_t* echo_create_rec(unsigned long stime){
	if (    ECHO_ECHO_BUFF!=NULL &&
            ECHO_ECHO_BUFF->LW != NULL &&
            ECHO_ECHO_BUFF->LW->latest_gvel_upd_at_create == lcosb_lame_getLastGvelUpdate() &&
		    ECHO_ECHO_BUFF->LW->echo_bundle.size < 15
        ) {
            #if LCOSB_DEBUG_LVL > LCOSB_ERR
            {   // Debug logs
                Serial.println("ECHO_ECHO_BUFF->LW can take more echo_record. Sending it back.");
            }
            #endif
		    return ECHO_ECHO_BUFF->LW;
        }
    
    #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
    {   // Debug logs
        Serial.println("ECHO_ECHO_BUFF is null or ECHO_ECHO_BUFF->LW is at max echo_record.");
    }
    #endif

	echo_record_t* newrec = (echo_record_t*) malloc(sizeof(echo_record_t));
	if(newrec) {
	newrec->echo_bundle.s_gtime = stime/1000;
	newrec->echo_bundle.e_gtime = stime/1000;
    // wtf is this noob mistake shit
	// newrec->echo_bundle.l = {-1};
	// newrec->echo_bundle.r = {-1};
	newrec->echo_bundle.size = 0;
	getGPos(newrec->echo_bundle.unit_pos.gpos);
	getGVel(newrec->echo_bundle.unit_pos.gvel);
    newrec->latest_gvel_upd_at_create = lcosb_lame_getLastGvelUpdate()/1000;
    newrec->next = NULL;
	} else {
        #if LCOSB_DEBUG_LVL > LCOSB_ERR
        {   // Debug logs
            Serial.println("NO MEMORY for echo_record_t* newrec. Sending NULL");
        }
        #endif
    }
	return newrec;
}
void printEchoBuffState() {
    #if LCOSB_DEBUG_LVL > LCOSB_ECHO_DEBUG

    {   // Debug logs
        Serial.println("Printing Echo Buffer State...");
    }

    echo_rec_buff_t* eb = ECHO_ECHO_BUFF;

    if (eb == NULL) {
        {   // Debug logs
            Serial.println("Echo Buffer is NULL.");
            Serial.println("Echo Buffer State printing completed.");
        }
        return;
    }

    {   // Debug logs
        Serial.print("Last Write NODE: ");
        Serial.println(reinterpret_cast<uintptr_t>(eb->LW), 16);
        Serial.print("Next Read NODE: ");
        Serial.println(reinterpret_cast<uintptr_t>(eb->NR), 16);
        Serial.print("Dropped Records: ");
        Serial.println(eb->_dropped);
        Serial.print("Buffer Size: ");
        Serial.println(eb->_size);
        Serial.print("Leaked Records: ");
        Serial.println(eb->_leak);
    }

    {   // Debug logs
        Serial.println("Printing Echo Records...");
    }

    {   // Debug logs for Next Read
        Serial.print("NR Address: ");
        Serial.println(reinterpret_cast<uintptr_t>(eb->NR), 16);
        Serial.print("gtime_e: ");
        Serial.print(eb->NR->gtime_e);
        // Serial.print(  "Left: ");
        // Serial.print(eb->NR->d_l);
        // Serial.print("  Right: ");
        // Serial.println(eb->NR->d_r);
        Serial.print("NR Next Record: ");
        Serial.println(reinterpret_cast<uintptr_t>(eb->NR->next), 16);
    }

    {   // Debug logs for Last Write
        Serial.print("LW Address: ");
        Serial.println(reinterpret_cast<uintptr_t>(eb->LW), 16);
        Serial.print("gtime_e: ");
        Serial.print(eb->LW->gtime_e);
        // Serial.print(  "Left: ");
        // Serial.print(eb->LW->d_l);
        // Serial.print("  Right: ");
        // Serial.println(eb->LW->d_r);
        Serial.print("LW Next Record: ");
        Serial.println(reinterpret_cast<uintptr_t>(eb->LW->next), 16);
    }

    {   // Debug logs for Next Read's Next
        if (eb->NR->next != NULL) {
            Serial.print("NR->next Address: ");
            Serial.println(reinterpret_cast<uintptr_t>(eb->NR->next), 16);
            Serial.print("gtime_e: ");
            Serial.print(eb->NR->next->gtime_e);
            // Serial.print(  "Left: ");
            // Serial.print(eb->NR->next->d_l);
            // Serial.print("  Right: ");
            // Serial.println(eb->NR->next->d_r);
            Serial.print("NR->next Next Record: ");
            Serial.println(reinterpret_cast<uintptr_t>(eb->NR->next->next), 16);
        }

        // Debug logs for Next Write's Next
        if (eb->LW->next != NULL) {
            Serial.print("LW->next Address: ");
            Serial.println(reinterpret_cast<uintptr_t>(eb->LW->next), 16);
            Serial.print("gtime_e: ");
            Serial.print(eb->LW->next->gtime_e);
            // Serial.print(  "Left: ");
            // Serial.print(eb->LW->next->d_l);
            // Serial.print("  Right: ");
            // Serial.println(eb->LW->next->d_r);
            Serial.print("LW->next Next Record: ");
            Serial.println(reinterpret_cast<uintptr_t>(eb->LW->next->next), 16);
        }
    }

    {   // Debug logs
        Serial.println("............. Echo Buffer State printing completed.");
    }

    #endif
}



// debug-testing
int echoBuffWriteObj(echo_record_t* rec) {
    if (rec == NULL) {
		#if LCOSB_DEBUG_LVL > LCOSB_ERR
        Serial.println("Received NULL pointer. Exiting[2]");
        #endif
        return 2;
    } 
	#if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
	else { 
		// debug log
        Serial.print("Good Pointer Recieved, rec (");
        Serial.print(reinterpret_cast<uintptr_t>(rec), 16);
        Serial.println(")");
    }
	#endif

    echo_rec_buff_t* eb = ECHO_ECHO_BUFF;
    if (eb == NULL) {
        #if LCOSB_DEBUG_LVL > LCOSB_ERR
        	Serial.println("ECHO_ECHO_BUFF is NULL pointer. call initEchoBuff(). Exiting[2]");
        #endif
        return 2;
    }

    if (eb->LW == NULL) { // new buffer
        #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
            Serial.println("New buffer Initializing...");
            Serial.println("Setting LW to rec");
        #endif
        eb->LW = rec;

        eb->NR = eb->LW;
        eb->LW->next = eb->NR;
        eb->_size = 1;

        return 3;
    }

    if (eb->_size < ECHO_BUFFER_SIZE_AVG || eb->LW->next == eb->NR) { // append to buffer
        #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
            Serial.println("Appending to buffer...");
            Serial.print("Appending to LW (");
            Serial.print(reinterpret_cast<uintptr_t>(eb->LW), 16);
            Serial.println(") to existing buffer.");
        #endif
        // maintained circular chain
        rec->next = eb->LW->next;
        // break prev loopover pointer
        eb->LW->next = rec;

        // update LW, size
        eb->LW = rec;
        eb->_size += 1;

        return 0;
    } else { // overwrite condition
        #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
            Serial.println("Overwriting buffer...");
            Serial.print("Overwriting record LW->next (");
            Serial.print(reinterpret_cast<uintptr_t>(eb->LW->next), 16);
            Serial.println(") in existing buffer.");
        #endif
        echo_record_t* orec = eb->LW->next;

        // copy data
        orec->echo_bundle = rec->echo_bundle;
        orec->gtime_e = rec->gtime_e;
        orec->latest_gvel_upd_at_create = rec->latest_gvel_upd_at_create;

        // update eb
        eb->LW = orec;
        eb->_leak += 1;

        if (eb->_size > 2 * ECHO_BUFFER_SIZE_AVG) {
            #if LCOSB_DEBUG_LVL > LCOSB_ERR
                Serial.println("Cleaning buffer due to overflow...");
            #endif
            echoBuffClean();
        }
        return 1;
    }

    #if LCOSB_DEBUG_LVL > LCOSB_ERR
        Serial.println("Exiting with unknown condition.");
    #endif
    return -1;
}

int echoBuffRead(lcosb_echo_bundle_t* data) { // data should be allocated mem already
  echo_rec_buff_t* eb = ECHO_ECHO_BUFF;

  if(eb->NR == NULL)    return 2;

  *data = eb->NR->echo_bundle;

  eb->NR = eb->NR->next;

  return 0;
}



void destroy(echo_rec_buff_t *eb) {
  if (!eb) return;

  int clear=0, drop=0;

  while(eb->LW->next != eb->LW) {
    echo_record_t* tmp = eb->LW->next;
    eb->LW->next = tmp->next;
    eb->_size -=1;

    if (tmp == eb->NR) drop = clear;
    
    free(tmp);
    clear +=1;
  }

  if (drop) eb->_dropped += clear - drop;

  eb->NR = eb->LW;
  eb->NR = echo_create_rec(millis());
  eb->_leak=0, eb->_size=1;
}

void echoBuffClean() {
  if (!ECHO_ECHO_BUFF) return;
  echo_rec_buff_t* eb = ECHO_ECHO_BUFF;

  int clear=0, drop=0;

  while(eb->_size > ECHO_BUFFER_SIZE_AVG) {
    echo_record_t* tmp = eb->LW->next;
    eb->LW->next = tmp->next;
    eb->_size -=1;

    if (tmp == eb->NR) drop = clear;
    
    free(tmp);
    clear +=1;
  }

  if (drop) {
    eb->_dropped += clear - drop;
    eb->NR = eb->LW->next;
  }
}

// debug-testing
void recordEcho(lcosb_echo_t* copy) {
    echo_record_t* nrec = NULL;
    nrec = echo_create_rec(millis());
	 if (nrec == NULL && copy == NULL) {
        #if LCOSB_DEBUG_LVL > LCOSB_ERR // Debug logs
        Serial.println("Both nrec and copy are NULL. Exiting.");
		#endif
        return;
    } 
	#if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
	else { // debuglog
        Serial.print("Pointers GOOD\nnrec=(");
        Serial.print(reinterpret_cast<uintptr_t>(nrec), 16);
        Serial.print(")\ncopy=(");
        Serial.print(reinterpret_cast<uintptr_t>(copy), 16);
        Serial.println(")");
    }
	#endif

    ulong_t stime = millis();

	#if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
    // Debug logs
    Serial.println("Triggering first ultrasonic sensor...");
	#endif

	// Triggering the first ultrasonic sensor
	digitalWrite(ECHO_TRIGGER, LOW);
	delayMicroseconds(2);
	digitalWrite(ECHO_TRIGGER, HIGH);
	delayMicroseconds(10);
	digitalWrite(ECHO_TRIGGER, LOW);


    ulong_t d_l = pulseInLong(ECHO_PIN_0, HIGH, ECHO_TIMEOUT_MAX);

    
    #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
    Serial.println("Triggering second ultrasonic sensor...");
    #endif

	// Triggering the second ultrasonic sensor
	digitalWrite(ECHO_TRIGGER, LOW);
	delayMicroseconds(2);
	digitalWrite(ECHO_TRIGGER, HIGH);
	delayMicroseconds(10);
	digitalWrite(ECHO_TRIGGER, LOW);
    

    ulong_t d_r = pulseInLong(ECHO_PIN_0, HIGH, ECHO_TIMEOUT_MAX);

    ulong_t avgtime = millis() >> 1 + stime >> 1;

    // Setting values in nrec if it exists
    if (nrec) {
		//int acctime = nrec->echo_bundle.gtime;
		int acclen  = nrec->echo_bundle.size;

        //nrec->echo_bundle.gtime = (acctime*acclen + avgtime/1000)/(acclen+1);
        nrec->echo_bundle.e_gtime = avgtime/1000;
        
        nrec->echo_bundle.l[acclen] = DL2MM_CALC(d_l);
        nrec->echo_bundle.r[acclen] = DL2MM_CALC(d_r)*-1; // -1 for slope and +-y axis deviation;
		nrec->echo_bundle.size = acclen+1;

        nrec->gtime_e = avgtime;


        #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
            Serial.print("Values set in nrec: s_gtime=");
            Serial.print(nrec->echo_bundle.s_gtime);
            Serial.print(", d_l[size]=");
            Serial.print(nrec->echo_bundle.l[acclen]);
            Serial.print(", d_r[size]=");
            Serial.print(nrec->echo_bundle.r[acclen]);
            Serial.print(", size[]=");
            Serial.println(nrec->echo_bundle.size);
		#endif
    }

    // Setting values in copy if it exists
    if (copy) {
        copy->ctime = avgtime;
        copy->left  = DL2MM_CALC(d_l);
        copy->right = DL2MM_CALC(d_r)*-1; // -1 for slope and +-y axis deviation

        #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
            Serial.print("Values set in copy: ctime=");
            Serial.print(copy->ctime);
            Serial.print(", d_l=");
            Serial.print(copy->left);
            Serial.print(", d_r=");
            Serial.println(copy->right);
        #endif
    }

    // Writing nrec to buffer and freeing memory
    if (nrec) {
        #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
            Serial.print("Writing nrec to buffer;");
        #endif
        int overwrite_flag = echoBuffWriteObj(nrec);
        #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
            Serial.print("ret-stat[echoBuffWriteObj]: ");
            Serial.println(overwrite_flag);
        #endif

        if (overwrite_flag==1) {
            free(nrec);
            #if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
                Serial.println("Overwrite received. nrec freed");
            #endif 
        }
    }
    
}
