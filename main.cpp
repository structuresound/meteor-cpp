#include "ddp.h"

using namespace std;

int initDB();

int main(int argc, char* argv[]) {
        initDB();

        DDPClient* ddpClient = new DDPClient();

        ddpClient->onConnected([ddpClient](const string& session){
            cout << "connected to " << ddpClient->hostname() << " " << session << endl;
            ddpClient->subscribe("offerings");
        });

        ddpClient->connect("wss://chroma.fund/websocket");
        //ddpClient->connect("ws://todos.meteor.com/websocket");
}

#include <ejdb.h>
#include <locale.h>

static EJDB *db;

int initDB() {
        setlocale(LC_ALL, "en_US.UTF-8");
        db = ejdbnew();
        if (!ejdbopen(db, "addressbook", JBOWRITER | JBOCREAT | JBOTRUNC)) {
                return 1;
        }

        //Get or create collection 'contacts'
        EJCOLL *coll = ejdbcreatecoll(db, "contacts", NULL);

        bson bsrec;
        bson_oid_t oid;

        //One record
        bson_init(&bsrec);
        bson_append_string(&bsrec, "name", "John Travolta");
        bson_append_string(&bsrec, "phone", "333-222-333");
        bson_append_int(&bsrec, "age", 58);
        bson_finish(&bsrec);
        ejdbsavebson(coll, &bsrec, &oid);
        fprintf(stderr, "\nSaved Travolta");
        bson_destroy(&bsrec);


        //Another record
        bson_init(&bsrec);
        bson_append_string(&bsrec, "name", "Bruce Willis");
        bson_append_string(&bsrec, "phone", "222-333-222");
        bson_append_int(&bsrec, "age", 57);
        bson_finish(&bsrec);
        ejdbsavebson(coll, &bsrec, &oid);
        fprintf(stderr, "\nSaved Bruce Willis");
        bson_destroy(&bsrec);


        //Now select one record.
        //Query: {'name' : {'$begin' : 'Bru'}} //Name starts with 'Bru' string
        bson bq1;
        bson_init_as_query(&bq1);
        bson_append_start_object(&bq1, "name");
        bson_append_string(&bq1, "$begin", "Bru");
        bson_append_finish_object(&bq1);
        bson_finish(&bq1);
        EJQ *q1 = ejdbcreatequery(db, &bq1, NULL, 0, NULL);

        uint32_t count;
        TCLIST *res = ejdbqryexecute(coll, q1, &count, 0, NULL);
        fprintf(stderr, "\n\nRecords found: %d\n", count);

        for (int i = 0; i < TCLISTNUM(res); ++i) {
                void *bsdata = TCLISTVALPTR(res, i);
                bson_print_raw((const char *)bsdata, 0);
        }
        fprintf(stderr, "\n");

        //Dispose result set
        tclistdel(res);

        //Dispose query
        ejdbquerydel(q1);
        bson_destroy(&bq1);

        //Close database
        ejdbclose(db);
        ejdbdel(db);
        return 0;
}
