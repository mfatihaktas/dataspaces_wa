/*
 Class           : GlobusCallback

 Description     :  Class contains  callback functions.

 Methods         : isDone()-> check the value of the done variable;
                    setDoneValue() -> Set the value of the done variable to true.
                    continueOnCond()
                    waitOnCond()

*/

class GlobusCallback {

protected:
        globus_mutex_t mutex;
        globus_cond_t cond;
   globus_bool_t done;
public :
        GlobusCallback(){

                globus_mutex_init(&mutex, GLOBUS_NULL);
                globus_cond_init(&cond, GLOBUS_NULL);
                done = GLOBUS_FALSE ;

        }

        ~GlobusCallback(){

                globus_mutex_destroy(&mutex);
                globus_cond_destroy(&cond);
        }

        globus_bool_t  isDone();
        void setDoneValue();
  void continueOnCond();
        void waitOnCond();

};

void GlobusCallback::waitOnCond() {

        globus_mutex_lock(&mutex);
        while(!isDone())
                globus_cond_wait(&cond,&mutex);
        globus_mutex_unlock(&mutex);
}
void GlobusCallback::continueOnCond() {

        globus_mutex_lock(&mutex);
        done= GLOBUS_FALSE;
        globus_mutex_unlock(&mutex);

}

void GlobusCallback::setDoneValue() {

        globus_mutex_lock(&mutex);
        done=GLOBUS_TRUE;
        globus_cond_signal(&cond);
        globus_mutex_unlock(&mutex);

}

globus_bool_t GlobusCallback::isDone() {

        return done;
}
