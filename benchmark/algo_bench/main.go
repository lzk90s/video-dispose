package main

import (
	"fmt"
	"net/http"
	"os"
	"io/ioutil"
	"encoding/json"
	"bytes"
	"encoding/base64"
	"time"
	"sync"
	"net"
	"strconv"
)


const (
	MaxIdleConns int = 10
	MaxIdleConnsPerHost int = 10
	IdleConnTimeout time.Duration = 90
)

func createHTTPClient() *http.Client {
	client := &http.Client{
		Transport: &http.Transport{
			Proxy: http.ProxyFromEnvironment,
			DialContext: (&net.Dialer{
				Timeout:   30 * time.Second,
				KeepAlive: 30 * time.Second,
			}).DialContext,
			MaxIdleConns:        MaxIdleConns,
			MaxIdleConnsPerHost: MaxIdleConnsPerHost,
			IdleConnTimeout:	 IdleConnTimeout * time.Second,
		},
	}
	return client
}

func readAll(filePth string) ([]byte, error) {
	f, err := os.Open(filePth)
	if err != nil {
		return nil, err
	}
	return ioutil.ReadAll(f)
}

func trace(msg string) func() {
	start := time.Now()
	//fmt.Printf("enter %s\n", msg)
	return func() {
		fmt.Printf("%s 耗时 (%s)\n", msg, time.Since(start))
	}
}

func runSeemmoAlgo(client *http.Client, url string, filePath string)   {
	imgData, err := readAll(filePath)
	if err != nil {
		return
	}

	type RecognizeRequest struct {
		Jpeg	string `json:"jpeg"`
	}

	recReq := &RecognizeRequest{
		Jpeg: base64.StdEncoding.EncodeToString(imgData),
	}

	data, err := json.Marshal(recReq)
	if err != nil {
		fmt.Printf("json.marshal failed, err:", err)
		return
	}

	resp, err := client.Post( url, "application/json", bytes.NewReader(data))
	defer resp.Body.Close()
	if err != nil {
		panic(err)
	}

	_, err = ioutil.ReadAll(resp.Body)
	if err != nil {
		// handle error
	}
	//fmt.Println(string(body))
}

func runGosunAlgo(client *http.Client, url string, filePath string)   {
	imgData, err := readAll(filePath)
	if err != nil {
		return
	}

	type RecognizeRequest struct {
		ImageData		string `json:"imageData"`
		CalcParam		string `json:"calcParam"`
		PictureFormat  	string `json:"pictureFormat"`
	}

	recReq := &RecognizeRequest{
		CalcParam:`{"levelWidth":40}`,
		PictureFormat:"jpg",
		ImageData: base64.StdEncoding.EncodeToString(imgData),
	}

	data, err := json.Marshal(recReq)
	if err != nil {
		fmt.Printf("json.marshal failed, err:", err)
		return
	}

	resp, err := client.Post( url, "application/json", bytes.NewReader(data))
	defer resp.Body.Close()
	if err != nil {
		fmt.Println("http post error", err)
		panic(err)
	}

	_, err = ioutil.ReadAll(resp.Body)
	if err != nil {
		// handle error
	}
}

type Callback func (client *http.Client, url string, filePath string)

type AlgoCaller struct {
	name 	string
	url 	string
	fn 		Callback
}


func main() {
	client := createHTTPClient()
	jpgPath := "f:\\result.jpg"
	times := []int {1, 10, 100,500}

	for _, t := range times {
		func (n int){
			algos := []AlgoCaller{
				AlgoCaller{name: "深瞐算法", fn: runSeemmoAlgo, url: "http://172.18.18.138:7000/ImgProcService/Recognize"},
				AlgoCaller{name: "高创算法", fn: runGosunAlgo, url: "http://172.18.18.138:7100/ImgProcService/Recognize"},
			}

			for _, algo := range algos{
				fmt.Printf("------------------ %s ------------------\n", algo.name)
				func (){
					url := algo.url
					fn  := algo.fn

					func (){
						defer trace("单线程循环" + strconv.Itoa(t) + "次")()
						for i:=0; i<t; i++{
							fn(client, url, jpgPath)
						}
					}()

					func (){
						defer trace("并发循环" + strconv.Itoa(t) + "次")()
						var wg sync.WaitGroup
						for i:= 0; i<t; i++{
							wg.Add(1)
							go func(){
								defer wg.Add(-1)
								fn(client, url, jpgPath)
							}()
						}
						wg.Wait()
					}()
				}()
			}
		}(t)
	}

}