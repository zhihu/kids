package kids

import (
    "strings"
    "testing"
)

func TestLog(t *testing.T) {
    var kids *Kids
    var err error
    if kids, err = Dial("tcp", "localhost:3388"); err != nil {
        t.Errorf(err.Error())
        return
    }
    kids.Log("kids.go.test", []byte("lalala"));
    kids.Close()
}

func TestSub(t *testing.T) {
    var kids *Kids
    var err error
    var resp [][]byte
    if kids, err = Dial("tcp", "localhost:3388"); err != nil {
        t.Errorf(err.Error())
        return
    }

    if resp, err = kids.Subscribe("kids.go.test"); err != nil {
        t.Errorf(err.Error())
    }

    if strings.ToUpper(string(resp[0])) != "SUBSCRIBE" {
        t.Errorf("Expected 'SUBSCRIBE' but got '%s'", strings.ToUpper(string(resp[0])))
    }
    if string(resp[1]) != "kids.go.test" {
        t.Errorf("Expected 'kids.go.test' but got '%s'", string(resp[1]))
    }
    if string(resp[2]) != "1" {
        t.Errorf("Expected '1' but got '%s'", string(resp[2]))
    }
    if err = kids.Log("kids.go.test", []byte("lalala")); err != nil {
        t.Errorf(err.Error())
    }

    log := <-kids.Logs
    if log.Topic != "kids.go.test" {
        t.Errorf("expect topic %s, but got %s", "kids.go.test", log.Topic)
    }
    kids.Close()
}

func TestPsub(t *testing.T) {
    var kids *Kids
    var err error
    var resp [][]byte
    if kids, err = Dial("tcp", "localhost:3388"); err != nil {
        t.Errorf(err.Error())
        return
    }

    if resp, err = kids.Psubscribe("kids.go.*"); err != nil {
        t.Errorf(err.Error())
    }

    if strings.ToUpper(string(resp[0])) != "PSUBSCRIBE" {
        t.Errorf("Expected 'PSUBSCRIBE' but got '%s'",strings.ToUpper(string(resp[0])))
    }
    if string(resp[1]) != "kids.go.*" {
        t.Errorf("Expected 'kids.go.*' but got '%s'", string(resp[1]))
    }
    if string(resp[2]) != "1" {
        t.Errorf("Expected '1' but got '%s'",strings.ToUpper(string(resp[2])))
    }
    if err = kids.Log("kids.go.test", []byte("lalala")); err != nil {
        t.Errorf(err.Error())
    }

    log := <-kids.Logs
    if log.Topic != "kids.go.test" {
        t.Errorf("expect topic %s, but got %s", "kids.go.test", log.Topic)
    }
    kids.Close()
}
